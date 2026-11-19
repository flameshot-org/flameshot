// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

// macOS backend: the Window Server lists the windows, and the Accessibility
// API provides the UI elements inside them.

#include "smartregiondetector.h"

#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CoreGraphics.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace {
// Accessibility calls run on the GUI thread, so a slow or hung application
// must not freeze the overlay. These are best-effort limits: macOS applies
// them per message, and the budget is checked between messages.
constexpr qint64 LOOKUP_BUDGET_MS = 200;
constexpr qint64 MESSAGE_TIMEOUT_MS = 80;
constexpr int MAX_DEPTH = 32;
constexpr int MIN_WINDOW_SIDE = 20;

// Owns a Core Foundation reference.
template<typename T>
class CFRef
{
public:
    explicit CFRef(T value = nullptr)
      : m_value(value)
    {}
    ~CFRef()
    {
        if (m_value) {
            CFRelease(m_value);
        }
    }
    CFRef(const CFRef&) = delete;
    CFRef& operator=(const CFRef&) = delete;

    T get() const { return m_value; }
    T* out() { return &m_value; }

    void reset(T value)
    {
        if (m_value) {
            CFRelease(m_value);
        }
        m_value = value;
    }

private:
    T m_value;
};

class LookupBudget
{
public:
    LookupBudget() { m_elapsed.start(); }

    // Applies the remaining budget as the element's messaging timeout.
    // Returns false once the budget is spent or a message has failed.
    bool prepare(AXUIElementRef element)
    {
        const qint64 remainingMs = LOOKUP_BUDGET_MS - m_elapsed.elapsed();
        if (m_failed || remainingMs <= 0) {
            return false;
        }
        const float timeout =
          static_cast<float>(std::min(remainingMs, MESSAGE_TIMEOUT_MS)) /
          1000.F;
        m_failed =
          AXUIElementSetMessagingTimeout(element, timeout) != kAXErrorSuccess;
        return !m_failed;
    }

    AXError record(AXError result)
    {
        m_failed = m_failed || result == kAXErrorCannotComplete ||
                   result == kAXErrorInvalidUIElement;
        return result;
    }

    bool exhausted() const
    {
        return m_failed || m_elapsed.elapsed() >= LOOKUP_BUDGET_MS;
    }

private:
    QElapsedTimer m_elapsed;
    bool m_failed = false;
};

AXError copyAttribute(AXUIElementRef element,
                      CFStringRef attribute,
                      CFTypeRef* value,
                      LookupBudget& budget)
{
    if (!budget.prepare(element)) {
        return kAXErrorCannotComplete;
    }
    return budget.record(
      AXUIElementCopyAttributeValue(element, attribute, value));
}

QRect toQRect(const CGRect& rect)
{
    const int left = static_cast<int>(std::floor(CGRectGetMinX(rect)));
    const int top = static_cast<int>(std::floor(CGRectGetMinY(rect)));
    const int right = static_cast<int>(std::ceil(CGRectGetMaxX(rect)));
    const int bottom = static_cast<int>(std::ceil(CGRectGetMaxY(rect)));
    return { left, top, right - left, bottom - top };
}

template<typename T>
bool axValue(CFTypeRef value, AXValueType type, T* result)
{
    return value && CFGetTypeID(value) == AXValueGetTypeID() &&
           AXValueGetValue(static_cast<AXValueRef>(value), type, result);
}

bool copyFrame(AXUIElementRef element, QRect* frame, LookupBudget& budget)
{
    CFRef<CFTypeRef> position;
    CFRef<CFTypeRef> size;
    CGPoint point;
    CGSize dimensions;
    if (copyAttribute(element, kAXPositionAttribute, position.out(), budget) !=
          kAXErrorSuccess ||
        copyAttribute(element, kAXSizeAttribute, size.out(), budget) !=
          kAXErrorSuccess ||
        !axValue(position.get(), kAXValueTypeCGPoint, &point) ||
        !axValue(size.get(), kAXValueTypeCGSize, &dimensions)) {
        return false;
    }
    *frame = toQRect(
      CGRectMake(point.x, point.y, dimensions.width, dimensions.height));
    return true;
}

// Returns the element's role, or nullptr. The caller owns the result.
CFStringRef copyRole(AXUIElementRef element, LookupBudget& budget)
{
    CFTypeRef value = nullptr;
    if (copyAttribute(element, kAXRoleAttribute, &value, budget) !=
          kAXErrorSuccess ||
        !value || CFGetTypeID(value) != CFStringGetTypeID()) {
        if (value) {
            CFRelease(value);
        }
        return nullptr;
    }
    return static_cast<CFStringRef>(value);
}

bool sameString(CFStringRef value, CFStringRef expected)
{
    return value && CFStringCompare(value, expected, 0) == kCFCompareEqualTo;
}

template<typename T>
bool dictionaryNumber(CFDictionaryRef dictionary,
                      CFStringRef key,
                      CFNumberType type,
                      T* value)
{
    const auto* number =
      static_cast<CFNumberRef>(CFDictionaryGetValue(dictionary, key));
    return number && CFGetTypeID(number) == CFNumberGetTypeID() &&
           CFNumberGetValue(number, type, value);
}

class MacOsSmartRegionDetector : public SmartRegionDetector
{
public:
    void snapshotWindows() override;
    QList<QRect> regionsAt(const QPoint& globalPosition) override;
    bool elementDetectionAvailable() const override { return m_trusted; }

private:
    struct Window
    {
        QRect geometry;
        pid_t pid;
    };

    QList<QRect> elementsAt(pid_t pid, const QPoint& position);
    void enableElectronAccessibility(AXUIElementRef application, pid_t pid);

    // Front to back, as reported by the Window Server.
    QList<Window> m_windows;
    QSet<pid_t> m_preparedApplications;
    bool m_trusted = false;
};

void MacOsSmartRegionDetector::snapshotWindows()
{
    m_windows.clear();
    m_trusted = AXIsProcessTrusted();

    CFRef<CFArrayRef> windows(CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
      kCGNullWindowID));
    if (!windows.get()) {
        return;
    }

    const auto ownPid = static_cast<pid_t>(QCoreApplication::applicationPid());
    for (CFIndex i = 0, count = CFArrayGetCount(windows.get()); i < count;
         ++i) {
        const auto* info = static_cast<CFDictionaryRef>(
          CFArrayGetValueAtIndex(windows.get(), i));
        int pid = 0;
        int layer = 0;
        double alpha = 1.0;
        CGRect bounds;
        const auto* boundsInfo = static_cast<CFDictionaryRef>(
          CFDictionaryGetValue(info, kCGWindowBounds));
        // Layer 0 holds normal application windows. Higher layers are the
        // menu bar, the Dock, status items and similar system surfaces.
        if (!dictionaryNumber(
              info, kCGWindowOwnerPID, kCFNumberIntType, &pid) ||
            pid == ownPid ||
            !dictionaryNumber(info, kCGWindowLayer, kCFNumberIntType, &layer) ||
            layer != 0 || !boundsInfo ||
            !CGRectMakeWithDictionaryRepresentation(boundsInfo, &bounds)) {
            continue;
        }
        dictionaryNumber(info, kCGWindowAlpha, kCFNumberDoubleType, &alpha);
        const QRect geometry = toQRect(bounds);
        if (alpha <= 0.01 || geometry.width() < MIN_WINDOW_SIDE ||
            geometry.height() < MIN_WINDOW_SIDE) {
            continue;
        }
        m_windows.append({ geometry, static_cast<pid_t>(pid) });
    }
}

QList<QRect> MacOsSmartRegionDetector::regionsAt(const QPoint& globalPosition)
{
    const auto window = std::find_if(
      m_windows.cbegin(), m_windows.cend(), [&](const Window& candidate) {
          return candidate.geometry.contains(globalPosition);
      });
    if (window == m_windows.cend()) {
        return {};
    }
    const QList<QRect> elements =
      m_trusted ? elementsAt(window->pid, globalPosition) : QList<QRect>();
    return buildRegionChain(elements, window->geometry, globalPosition);
}

QList<QRect> MacOsSmartRegionDetector::elementsAt(pid_t pid,
                                                  const QPoint& position)
{
    LookupBudget budget;
    // Hit-test inside the application that owns the window. The system-wide
    // element would find Flameshot's own overlay, which is now on top.
    CFRef<AXUIElementRef> application(AXUIElementCreateApplication(pid));
    if (!application.get() || !budget.prepare(application.get())) {
        return {};
    }
    enableElectronAccessibility(application.get(), pid);

    CFRef<AXUIElementRef> element;
    if (budget.record(
          AXUIElementCopyElementAtPosition(application.get(),
                                           static_cast<float>(position.x()),
                                           static_cast<float>(position.y()),
                                           element.out())) != kAXErrorSuccess) {
        return {};
    }

    QList<QRect> elements;
    for (int depth = 0; element.get() && depth < MAX_DEPTH; ++depth) {
        const CFRef<CFStringRef> role(copyRole(element.get(), budget));
        if (budget.exhausted() || sameString(role.get(), kAXApplicationRole)) {
            break;
        }
        QRect frame;
        if (copyFrame(element.get(), &frame, budget)) {
            elements.append(frame);
        }
        if (budget.exhausted() || sameString(role.get(), kAXWindowRole)) {
            break;
        }
        CFTypeRef parent = nullptr;
        if (copyAttribute(element.get(), kAXParentAttribute, &parent, budget) !=
              kAXErrorSuccess ||
            !parent || CFGetTypeID(parent) != AXUIElementGetTypeID()) {
            if (parent) {
                CFRelease(parent);
            }
            break;
        }
        element.reset(static_cast<AXUIElementRef>(parent));
    }
    // A partial chain could miss the elements between the deepest one and
    // the window. Fall back to the window; a later lookup can fill it in.
    return budget.exhausted() ? QList<QRect>() : elements;
}

void MacOsSmartRegionDetector::enableElectronAccessibility(
  AXUIElementRef application,
  pid_t pid)
{
    // Electron applications only build their accessibility tree for
    // assistive tools that set this documented attribute. Other applications
    // reject it, which is harmless.
    if (!m_preparedApplications.contains(pid)) {
        m_preparedApplications.insert(pid);
        AXUIElementSetAttributeValue(
          application, CFSTR("AXManualAccessibility"), kCFBooleanTrue);
    }
}
} // namespace

bool nativeSmartRegionDetectorSupported()
{
    return true;
}

std::unique_ptr<SmartRegionDetector> createNativeSmartRegionDetector()
{
    return std::make_unique<MacOsSmartRegionDetector>();
}

bool macOsAccessibilityTrusted()
{
    return AXIsProcessTrusted();
}

void requestMacOsAccessibilityPermission()
{
    const void* keys[] = { kAXTrustedCheckOptionPrompt };
    const void* values[] = { kCFBooleanTrue };
    CFRef<CFDictionaryRef> options(
      CFDictionaryCreate(kCFAllocatorDefault,
                         keys,
                         values,
                         1,
                         &kCFTypeDictionaryKeyCallBacks,
                         &kCFTypeDictionaryValueCallBacks));
    AXIsProcessTrustedWithOptions(options.get());
}
