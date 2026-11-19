// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

// Windows backend: top-level windows come from EnumWindows and DWM, and the
// UI elements inside them from UI Automation.

#include "smartregiondetector.h"
#include "smartregiongeometry.h"
#include "utils/inputtrace.h"
#include "utils/windowsforeground.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
// UIAutomation requires Windows and OLE declarations in this order.
// clang-format off
#include <Windows.h>
#include <ole2.h>
#include <UIAutomation.h>
// clang-format on
#include <dwmapi.h>
#include <oleacc.h>
#include <wrl/client.h>

#include <QGuiApplication>
#include <QScreen>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cwchar>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {
using SmartRegionGeometry::ScreenMapping;

constexpr int MIN_WINDOW_SIDE = 20;

// How long the GUI thread waits for a lookup before showing the window, how
// long the worker may keep looking in the background, and how long closing
// the overlay waits for the worker.
constexpr std::chrono::milliseconds GUI_WAIT(60);
constexpr std::chrono::milliseconds SHUTDOWN_WAIT(500);
// How long showing the overlay waits for Chromium applications to be woken.
constexpr std::chrono::milliseconds WAKE_WAIT(200);
constexpr std::chrono::milliseconds REQUEST_BUDGET(1500);
// Per call into another application.
constexpr DWORD PROVIDER_TIMEOUT_MS = 300;
constexpr int MAX_DEPTH = 32;
constexpr int MAX_VISITED_ELEMENTS = 2048;
using SmartRegionGeometry::toLogicalRect;

struct MonitorEnumerationContext
{
    QList<ScreenMapping>* mappings;
    QList<QScreen*> unmatchedScreens;
};

QRect rectFromNative(const RECT& rect)
{
    return {
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top
    };
}

bool validNativeRect(const RECT& rect)
{
    return rect.right > rect.left && rect.bottom > rect.top;
}

bool containsPoint(const RECT& rect, const POINT& point)
{
    return point.x >= rect.left && point.x < rect.right &&
           point.y >= rect.top && point.y < rect.bottom;
}

BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM contextValue)
{
    auto* context = reinterpret_cast<MonitorEnumerationContext*>(contextValue);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return TRUE;
    }

    // Qt keeps screen origins in physical pixels on Windows, so the origin
    // identifies the screen; Qt 6 may report a friendly name instead of the
    // device name.
    const QString deviceName = QString::fromWCharArray(info.szDevice);
    const QPoint origin(info.rcMonitor.left, info.rcMonitor.top);
    auto screenIterator =
      std::find_if(context->unmatchedScreens.begin(),
                   context->unmatchedScreens.end(),
                   [&origin](QScreen* screen) {
                       return screen->geometry().topLeft() == origin;
                   });
    if (screenIterator == context->unmatchedScreens.end()) {
        screenIterator =
          std::find_if(context->unmatchedScreens.begin(),
                       context->unmatchedScreens.end(),
                       [&deviceName](QScreen* screen) {
                           return screen->name().compare(
                                    deviceName, Qt::CaseInsensitive) == 0;
                       });
    }
    if (screenIterator == context->unmatchedScreens.end()) {
        QScreen* primary = QGuiApplication::primaryScreen();
        screenIterator = std::find(context->unmatchedScreens.begin(),
                                   context->unmatchedScreens.end(),
                                   primary);
    }
    if (screenIterator == context->unmatchedScreens.end() &&
        !context->unmatchedScreens.isEmpty()) {
        screenIterator = context->unmatchedScreens.begin();
    }
    if (screenIterator == context->unmatchedScreens.end()) {
        return TRUE;
    }

    context->mappings->append(
      { rectFromNative(info.rcMonitor), (*screenIterator)->geometry() });
    context->unmatchedScreens.erase(screenIterator);
    return TRUE;
}

// Finds the UI elements under a point in another application's window.
//
// MSAA hit-testing asks the application itself what is at the point, one
// call per level, which stays fast in large trees such as web pages. When an
// application only answers with the window, UI Automation is searched
// instead, one level of children per call.
//
// Calls into other processes can be slow or hang, so they run on a thread of
// their own. The GUI thread only waits briefly; a slower lookup keeps running
// and its result is returned when the same window and position are asked for
// again, as the capture widget does while the pointer rests.
class AutomationWorker
{
public:
    AutomationWorker()
      : m_state(std::make_shared<State>())
    {
        std::thread([state = m_state]() { run(*state); }).detach();
    }

    ~AutomationWorker()
    {
        std::unique_lock lock(m_state->mutex);
        m_state->stopping = true;
        ++m_state->generation;
        m_state->wake.notify_one();
        // Wait for the worker to finish, unless an application hangs: the
        // worker owns its state, so it can finish on its own later.
        m_state->done.wait_for(
          lock, SHUTDOWN_WAIT, [this]() { return m_state->finishedRunning; });
    }

    // Element rectangles in physical pixels, from the deepest element under
    // the position to the window. Empty while the lookup is still running.
    QList<QRect> elementChainAt(HWND window, const POINT& position)
    {
        State& state = *m_state;
        const Request request{ window, position };
        std::unique_lock lock(state.mutex);
        if (state.finished == request) {
            return takeResult(state);
        }
        if (state.running != request && state.pending != request) {
            // A new position supersedes the lookup in progress.
            state.pending = request;
            ++state.generation;
            state.wake.notify_one();
        }
        state.done.wait_for(lock, GUI_WAIT, [&]() {
            return state.stopping || state.finished == request;
        });
        return state.finished == request ? takeResult(state) : QList<QRect>();
    }

    bool available() const { return m_state->available.load(); }

    // Chromium-based applications (Chrome, Edge, Electron) only build the
    // element tree of their web content once an assistive tool hit-tests
    // them through UI Automation. That hit-test would find the capture
    // overlay, so it is done at these points before the overlay is shown.
    // The applications then build the tree in about a second.
    void wakeChromiumAt(QList<POINT> points)
    {
        State& state = *m_state;
        std::unique_lock lock(state.mutex);
        state.wakePoints = std::move(points);
        state.wakePending = !state.wakePoints.isEmpty();
        state.wake.notify_one();
        state.done.wait_for(lock, WAKE_WAIT, [&state]() {
            return state.stopping || !state.wakePending;
        });
    }

private:
    using Clock = std::chrono::steady_clock;

    struct Request
    {
        HWND window;
        POINT position;
        bool operator==(const Request& other) const
        {
            return window == other.window && position.x == other.position.x &&
                   position.y == other.position.y;
        }
        bool operator!=(const Request& other) const
        {
            return !(*this == other);
        }
    };

    struct Lookup
    {
        Request request;
        quint64 generation;
        Clock::time_point deadline;
    };

    struct State
    {
        std::mutex mutex;
        std::condition_variable wake;
        std::condition_variable done;
        std::optional<Request> pending;
        std::optional<Request> running;
        std::optional<Request> finished;
        QList<QRect> result;
        bool stopping = false;
        bool finishedRunning = false;
        // Changes whenever a lookup is superseded, which cancels it.
        std::atomic<quint64> generation{ 0 };
        std::atomic_bool available{ false };

        // Only used on the worker thread.
        ComPtr<IUIAutomation2> automation;
        ComPtr<IUIAutomationCondition> controlView;
        ComPtr<IUIAutomationCacheRequest> cacheRequest;
        QList<POINT> wakePoints;
        bool wakePending = false;
    };

    // A result is handed out once: asking again starts a new lookup, which
    // finds elements an application has added since, e.g. once Chromium has
    // built its tree.
    static QList<QRect> takeResult(State& state)
    {
        state.finished.reset();
        return std::exchange(state.result, {});
    }

    static bool active(const State& state, const Lookup& lookup)
    {
        return state.generation.load() == lookup.generation &&
               Clock::now() < lookup.deadline;
    }

    // The innermost visible child window under the position.
    static HWND childWindowAt(HWND window, const POINT& position)
    {
        HWND current = window;
        for (int depth = 0; depth < MAX_DEPTH; ++depth) {
            POINT client = position;
            ScreenToClient(current, &client);
            const HWND child = ChildWindowFromPointEx(
              current,
              client,
              CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
            if (!child || child == current) {
                break;
            }
            current = child;
        }
        return current;
    }

    static bool location(IAccessible* accessible, LONG childId, QRect* rect)
    {
        VARIANT child;
        child.vt = VT_I4;
        child.lVal = childId;
        long left = 0, top = 0, width = 0, height = 0;
        if (FAILED(
              accessible->accLocation(&left, &top, &width, &height, child)) ||
            width <= 0 || height <= 0) {
            return false;
        }
        *rect = QRect(left, top, width, height);
        return true;
    }

    // MSAA: hit-test down to the deepest element, then collect it and its
    // parents up to the window.
    static QList<QRect> msaaChain(const State& state,
                                  const Lookup& lookup,
                                  HWND window)
    {
        const POINT& position = lookup.request.position;
        ComPtr<IAccessible> current;
        if (FAILED(AccessibleObjectFromWindow(childWindowAt(window, position),
                                              OBJID_CLIENT,
                                              IID_PPV_ARGS(&current))) ||
            !current) {
            return {};
        }

        QList<QRect> chain;
        for (int depth = 0; depth < MAX_DEPTH && active(state, lookup);
             ++depth) {
            VARIANT hit;
            VariantInit(&hit);
            if (FAILED(current->accHitTest(position.x, position.y, &hit))) {
                break;
            }
            if (hit.vt == VT_DISPATCH && hit.pdispVal) {
                ComPtr<IAccessible> next;
                hit.pdispVal->QueryInterface(IID_PPV_ARGS(&next));
                hit.pdispVal->Release();
                if (!next || next.Get() == current.Get()) {
                    break;
                }
                current = next;
                continue;
            }
            // A simple element without an object of its own.
            QRect rect;
            if (hit.vt == VT_I4 && hit.lVal != CHILDID_SELF &&
                location(current.Get(), hit.lVal, &rect)) {
                chain.append(rect);
            }
            VariantClear(&hit);
            break;
        }

        RECT windowRect{};
        GetWindowRect(window, &windowRect);
        const QRect windowGeometry = rectFromNative(windowRect);
        for (int depth = 0;
             current && depth < MAX_DEPTH && active(state, lookup);
             ++depth) {
            QRect rect;
            if (location(current.Get(), CHILDID_SELF, &rect)) {
                if (rect.contains(windowGeometry)) {
                    break;
                }
                chain.append(rect);
            }
            ComPtr<IDispatch> parentDispatch;
            ComPtr<IAccessible> parent;
            if (FAILED(current->get_accParent(&parentDispatch)) ||
                !parentDispatch || FAILED(parentDispatch.As(&parent))) {
                break;
            }
            current = parent;
        }
        return active(state, lookup) ? chain : QList<QRect>();
    }

    // Reads the bounding rectangle fetched with the element. Off-screen
    // elements and those not under the position are skipped.
    static bool cachedRectAt(IUIAutomationElement* element,
                             const POINT& position,
                             RECT* rect)
    {
        BOOL offscreen = TRUE;
        return SUCCEEDED(element->get_CachedBoundingRectangle(rect)) &&
               validNativeRect(*rect) && containsPoint(*rect, position) &&
               SUCCEEDED(element->get_CachedIsOffscreen(&offscreen)) &&
               !offscreen;
    }

    struct Search
    {
        const State& state;
        const Lookup& lookup;
        QList<QRect> path;
        QList<QRect> deepest;
        int visited = 0;
    };

    // UI Automation: siblings can overlap, so every branch under the position
    // is searched; the deepest element wins, and the smaller one on a tie.
    // Each level is fetched with its rectangles in a single call.
    static void descend(IUIAutomationElement* element, Search& search)
    {
        const auto area = [](const QRect& rect) {
            return qint64(rect.width()) * rect.height();
        };
        if (search.path.size() > search.deepest.size() ||
            (search.path.size() == search.deepest.size() &&
             area(search.path.last()) < area(search.deepest.last()))) {
            search.deepest = search.path;
        }
        if (search.path.size() >= MAX_DEPTH ||
            search.visited >= MAX_VISITED_ELEMENTS ||
            !active(search.state, search.lookup)) {
            return;
        }

        ComPtr<IUIAutomationElementArray> children;
        int count = 0;
        if (FAILED(element->FindAllBuildCache(TreeScope_Children,
                                              search.state.controlView.Get(),
                                              search.state.cacheRequest.Get(),
                                              &children)) ||
            !children || FAILED(children->get_Length(&count))) {
            return;
        }
        for (int i = 0; i < count && search.visited < MAX_VISITED_ELEMENTS &&
                        active(search.state, search.lookup);
             ++i, ++search.visited) {
            ComPtr<IUIAutomationElement> child;
            RECT rect{};
            if (SUCCEEDED(children->GetElement(i, &child)) && child &&
                cachedRectAt(
                  child.Get(), search.lookup.request.position, &rect)) {
                search.path.append(rectFromNative(rect));
                descend(child.Get(), search);
                search.path.removeLast();
            }
        }
    }

    static QList<QRect> automationChain(const State& state,
                                        const Lookup& lookup)
    {
        const Request& request = lookup.request;
        ComPtr<IUIAutomationElement> root;
        RECT rootRect{};
        if (FAILED(state.automation->ElementFromHandleBuildCache(
              request.window, state.cacheRequest.Get(), &root)) ||
            !root || !cachedRectAt(root.Get(), request.position, &rootRect)) {
            return {};
        }
        Search search{ state, lookup, { rectFromNative(rootRect) }, {} };
        descend(root.Get(), search);
        return QList<QRect>(search.deepest.crbegin(), search.deepest.crend());
    }

    // Returns false when the lookup ran out of time or was superseded.
    static bool lookUp(State& state, const Lookup& lookup, QList<QRect>* chain)
    {
        const HWND window = lookup.request.window;
        if (!IsWindow(window)) {
            chain->clear();
            return true;
        }

        // Prefer MSAA when it finds an element inside the innermost window.
        RECT childRect{};
        GetWindowRect(childWindowAt(window, lookup.request.position),
                      &childRect);
        *chain = msaaChain(state, lookup, window);
        const auto area = [](const QRect& rect) {
            return qint64(rect.width()) * rect.height();
        };
        if (chain->isEmpty() ||
            area(chain->first()) >= area(rectFromNative(childRect))) {
            *chain = automationChain(state, lookup);
        }
        return active(state, lookup);
    }

    static bool initialize(State& state)
    {
        // CUIAutomation8 provides the provider timeouts.
        return SUCCEEDED(CoCreateInstance(CLSID_CUIAutomation8,
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(&state.automation))) &&
               SUCCEEDED(state.automation->put_ConnectionTimeout(
                 PROVIDER_TIMEOUT_MS)) &&
               SUCCEEDED(state.automation->put_TransactionTimeout(
                 PROVIDER_TIMEOUT_MS)) &&
               SUCCEEDED(state.automation->get_ControlViewCondition(
                 &state.controlView)) &&
               SUCCEEDED(
                 state.automation->CreateCacheRequest(&state.cacheRequest)) &&
               SUCCEEDED(state.cacheRequest->AddProperty(
                 UIA_BoundingRectanglePropertyId)) &&
               SUCCEEDED(
                 state.cacheRequest->AddProperty(UIA_IsOffscreenPropertyId));
    }

    static void run(State& state)
    {
        const bool comReady =
          SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        state.available.store(comReady && initialize(state));

        std::unique_lock lock(state.mutex);
        for (;;) {
            state.wake.wait(lock, [&state]() {
                return state.stopping || state.pending || state.wakePending;
            });
            if (state.stopping) {
                break;
            }
            if (state.wakePending) {
                const QList<POINT> points = std::exchange(state.wakePoints, {});
                lock.unlock();
                for (const POINT& point : points) {
                    ComPtr<IUIAutomationElement> element;
                    if (state.available.load()) {
                        state.automation->ElementFromPoint(point, &element);
                    }
                }
                lock.lock();
                state.wakePending = false;
                state.done.notify_all();
                continue;
            }
            const Lookup lookup{ *state.pending,
                                 state.generation.load(),
                                 Clock::now() + REQUEST_BUDGET };
            state.running = state.pending;
            state.pending.reset();
            lock.unlock();

            QList<QRect> chain;
            const bool complete =
              state.available.load() && lookUp(state, lookup, &chain);

            lock.lock();
            state.running.reset();
            // Keep complete results only, so an expired lookup is retried.
            if (complete) {
                state.finished = lookup.request;
                state.result = std::move(chain);
            }
            state.done.notify_all();
        }
        lock.unlock();

        // Release apartment-bound interfaces before shutting down COM.
        state.cacheRequest.Reset();
        state.controlView.Reset();
        state.automation.Reset();
        if (comReady) {
            CoUninitialize();
        }
        lock.lock();
        state.finishedRunning = true;
        state.done.notify_all();
    }

    std::shared_ptr<State> m_state;
};

class WindowsSmartRegionDetector : public SmartRegionDetector
{
public:
    void snapshotWindows() override;
    QList<QRect> regionsAt(const QPoint& globalPosition) override;
    bool elementDetectionAvailable() const override
    {
        return m_automation.available();
    }

private:
    struct Window
    {
        HWND handle;
        QRect nativeGeometry;
    };

    static BOOL CALLBACK collectWindow(HWND window, LPARAM detector)
    {
        reinterpret_cast<WindowsSmartRegionDetector*>(detector)->addWindow(
          window);
        return TRUE;
    }

    void addWindow(HWND window);
    void wakeChromium();

    // Front to back, in the order EnumWindows reports them.
    QList<Window> m_windows;
    QList<ScreenMapping> m_screens;
    AutomationWorker m_automation;
};

void WindowsSmartRegionDetector::snapshotWindows()
{
    m_windows.clear();
    m_screens.clear();

    MonitorEnumerationContext context{ &m_screens, QGuiApplication::screens() };
    EnumDisplayMonitors(
      nullptr, nullptr, collectMonitor, reinterpret_cast<LPARAM>(&context));
    EnumWindows(collectWindow, reinterpret_cast<LPARAM>(this));
    wakeChromium();
}

void WindowsSmartRegionDetector::wakeChromium()
{
    constexpr int MAX_WINDOWS = 8;
    const auto isChromium = [](HWND window) {
        wchar_t name[32] = {};
        GetClassNameW(window, name, static_cast<int>(std::size(name)));
        return wcscmp(name, L"Chrome_WidgetWin_1") == 0;
    };
    // A point where the window itself is visible, so the hit-test reaches it.
    const auto visibleAt = [](HWND window, const POINT& point) {
        return GetAncestor(WindowFromPoint(point), GA_ROOT) == window;
    };

    QList<POINT> points;
    POINT cursor{};
    if (GetCursorPos(&cursor)) {
        const HWND window = GetAncestor(WindowFromPoint(cursor), GA_ROOT);
        if (window && isChromium(window)) {
            points.append(cursor);
        }
    }
    for (const Window& window : m_windows) {
        if (points.size() >= MAX_WINDOWS) {
            break;
        }
        const QPoint center = window.nativeGeometry.center();
        const POINT point{ center.x(), center.y() };
        if (isChromium(window.handle) && visibleAt(window.handle, point)) {
            points.append(point);
        }
    }
    if (!points.isEmpty()) {
        m_automation.wakeChromiumAt(std::move(points));
    }
}

void WindowsSmartRegionDetector::addWindow(HWND window)
{
    if (!IsWindowVisible(window) || IsIconic(window) ||
        GetAncestor(window, GA_ROOT) != window ||
        window == GetDesktopWindow() || window == GetShellWindow()) {
        return;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (processId == 0 || processId == GetCurrentProcessId()) {
        return;
    }

    // Cloaked windows are hidden by DWM, e.g. on another virtual desktop.
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(
          window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) &&
        cloaked != 0) {
        return;
    }

    const LONG_PTR extendedStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
    if ((extendedStyle & WS_EX_LAYERED) != 0) {
        // Layered windows that are click-through or fully transparent are
        // overlays the user cannot see or interact with.
        if ((extendedStyle & WS_EX_TRANSPARENT) != 0) {
            return;
        }
        BYTE alpha = 255;
        DWORD flags = 0;
        if (GetLayeredWindowAttributes(window, nullptr, &alpha, &flags) &&
            (flags & LWA_ALPHA) != 0 && alpha == 0) {
            return;
        }
    }

    // The extended frame bounds exclude the invisible resize borders that
    // GetWindowRect includes on Windows 10 and later.
    RECT nativeRect{};
    if (FAILED(DwmGetWindowAttribute(window,
                                     DWMWA_EXTENDED_FRAME_BOUNDS,
                                     &nativeRect,
                                     sizeof(nativeRect))) &&
        !GetWindowRect(window, &nativeRect)) {
        return;
    }
    if (!validNativeRect(nativeRect) ||
        nativeRect.right - nativeRect.left < MIN_WINDOW_SIDE ||
        nativeRect.bottom - nativeRect.top < MIN_WINDOW_SIDE) {
        return;
    }
    m_windows.append({ window, rectFromNative(nativeRect) });
}

QList<QRect> WindowsSmartRegionDetector::regionsAt(const QPoint& globalPosition)
{
    // Map through the pointer's screen only: with mixed DPI, parts of a
    // window on other screens would map to a different logical scale.
    const ScreenMapping* screen =
      SmartRegionGeometry::screenAt(globalPosition, m_screens);
    if (!screen) {
        return {};
    }
    const QPoint nativePoint =
      SmartRegionGeometry::toNativePoint(globalPosition, *screen);
    const auto window = std::find_if(
      m_windows.cbegin(), m_windows.cend(), [&](const Window& candidate) {
          return candidate.nativeGeometry.contains(nativePoint);
      });
    if (window == m_windows.cend()) {
        return {};
    }

    QList<QRect> elements;
    const POINT nativePosition{ nativePoint.x(), nativePoint.y() };
    for (const QRect& element :
         m_automation.elementChainAt(window->handle, nativePosition)) {
        elements.append(toLogicalRect(element, *screen));
    }
    return buildRegionChain(
      elements, toLogicalRect(window->nativeGeometry, *screen), globalPosition);
}
} // namespace

namespace {
// HID usage of precision touchpads.
constexpr USHORT DIGITIZER_USAGE_PAGE = 0x0D;
constexpr USHORT TOUCHPAD_USAGE = 0x05;

// Raw input works in this process once any report has arrived.
bool s_rawInputSeen = false;

// The precision touchpad's "Scrolling direction" setting. 0, the default,
// means that moving the fingers up scrolls the content down; other values
// reverse that. See "Precision touchpad tuning" in Microsoft's docs.
bool touchpadScrollIsNatural(bool* exists)
{
    DWORD direction = 0;
    DWORD size = sizeof(direction);
    *exists =
      RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\PrecisionTouchPad",
        L"ScrollDirection",
        RRF_RT_REG_DWORD,
        nullptr,
        &direction,
        &size) == ERROR_SUCCESS;
    return direction == 0;
}

bool precisionTouchpadPresent()
{
    UINT count = 0;
    if (GetRawInputDeviceList(nullptr, &count, sizeof(RAWINPUTDEVICELIST)) !=
          0 ||
        count == 0) {
        return false;
    }
    std::vector<RAWINPUTDEVICELIST> devices(count);
    const UINT found =
      GetRawInputDeviceList(devices.data(), &count, sizeof(RAWINPUTDEVICELIST));
    if (found == UINT(-1)) {
        return false;
    }
    for (UINT i = 0; i < found; ++i) {
        RID_DEVICE_INFO info{};
        info.cbSize = sizeof(info);
        UINT size = sizeof(info);
        const UINT copied = GetRawInputDeviceInfoW(
          devices[i].hDevice, RIDI_DEVICEINFO, &info, &size);
        if (devices[i].dwType == RIM_TYPEHID && copied != 0 &&
            copied != UINT(-1) &&
            info.hid.usUsagePage == DIGITIZER_USAGE_PAGE &&
            info.hid.usUsage == TOUCHPAD_USAGE) {
            return true;
        }
    }
    return false;
}

// Lets the Windows end-to-end test simulate touchpad contact, which cannot
// be injected, when FLAMESHOT_TEST_HOOKS is set.
UINT testContactMessage()
{
    static const UINT message =
      qEnvironmentVariableIsSet("FLAMESHOT_TEST_HOOKS")
        ? RegisterWindowMessageW(L"FlameshotTestTouchpadContact")
        : 0;
    return message;
}

QString reasonName(TouchpadScrollClassifier::Reason reason)
{
    using Reason = TouchpadScrollClassifier::Reason;
    switch (reason) {
        case Reason::Mouse:
            return QStringLiteral("mouse");
        case Reason::Source:
            return QStringLiteral("source");
        case Reason::Contact:
            return QStringLiteral("contact");
        case Reason::Sequence:
            return QStringLiteral("sequence");
        case Reason::Delta:
            return QStringLiteral("delta");
    }
    return {};
}

void logMessage(const MSG* msg)
{
    switch (msg->message) {
        case WM_ACTIVATE:
        case WM_NCACTIVATE:
        case WM_ACTIVATEAPP:
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            InputTrace::write(QStringLiteral("focus msg=0x%1 wparam=%2 %3")
                                .arg(msg->message, 0, 16)
                                .arg(msg->wParam)
                                .arg(WindowsForeground::describe()));
            break;
        case WM_KEYDOWN:
            if (msg->wParam == VK_ESCAPE) {
                InputTrace::write(QStringLiteral("key escape"));
            }
            break;
        case WM_GESTURE:
        case WM_VSCROLL:
            InputTrace::write(
              QStringLiteral("other msg=0x%1").arg(msg->message, 0, 16));
            break;
        default:
            if (msg->message >= 0x0245 && msg->message <= 0x024F) {
                POINTER_INPUT_TYPE type = PT_POINTER;
                GetPointerType(GET_POINTERID_WPARAM(msg->wParam), &type);
                InputTrace::write(QStringLiteral("pointer msg=0x%1 type=%2")
                                    .arg(msg->message, 0, 16)
                                    .arg(type));
            }
    }
}
}

TouchpadScrollTracker::~TouchpadScrollTracker()
{
    if (m_registered) {
        RAWINPUTDEVICE device{
            DIGITIZER_USAGE_PAGE, TOUCHPAD_USAGE, RIDEV_REMOVE, nullptr
        };
        RegisterRawInputDevices(&device, 1, sizeof(device));
    }
}

void TouchpadScrollTracker::attach(void* window)
{
    m_window = window;
    // The overlay receives wheel messages under the pointer even when it is
    // not the foreground window, so it needs the touchpad's reports then as
    // well, which RIDEV_INPUTSINK provides.
    RAWINPUTDEVICE device{ DIGITIZER_USAGE_PAGE,
                           TOUCHPAD_USAGE,
                           RIDEV_INPUTSINK,
                           static_cast<HWND>(window) };
    m_registered = RegisterRawInputDevices(&device, 1, sizeof(device)) != FALSE;
    const DWORD error = m_registered ? 0 : GetLastError();
    bool settingExists = false;
    m_naturalSetting = touchpadScrollIsNatural(&settingExists);
    const bool touchpad = precisionTouchpadPresent();
    m_classifier.setTouchpadPresent(touchpad);
    m_classifier.setRawInputWorks(s_rawInputSeen);
    if (InputTrace::enabled()) {
        InputTrace::write(
          QStringLiteral("attach hwnd=0x%1 registered=%2 error=%3 "
                         "touchpad=%4 scrollDirectionNatural=%5 "
                         "settingExists=%6 rawInputSeen=%7")
            .arg(reinterpret_cast<quintptr>(window), 0, 16)
            .arg(m_registered)
            .arg(error)
            .arg(touchpad)
            .arg(m_naturalSetting)
            .arg(settingExists)
            .arg(s_rawInputSeen));
    }
}

bool TouchpadScrollTracker::handleMessage(const void* message)
{
    const auto* msg = static_cast<const MSG*>(message);
    if (!msg->hwnd) {
        return false;
    }
    if (m_enabled && msg->hwnd != m_window) {
        // Also covers a window that has been recreated.
        attach(msg->hwnd);
    }
    if (InputTrace::enabled()) {
        logMessage(msg);
    }
    const auto time = static_cast<quint32>(GetMessageTime());

    const UINT testContact = testContactMessage();
    if (msg->message == WM_INPUT ||
        (testContact != 0 && msg->message == testContact)) {
        bool touchpadReport = msg->message != WM_INPUT;
        if (!touchpadReport) {
            RAWINPUTHEADER header{};
            UINT size = sizeof(header);
            touchpadReport =
              GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam),
                              RID_HEADER,
                              &header,
                              &size,
                              sizeof(header)) != UINT(-1) &&
              header.dwType == RIM_TYPEHID;
        }
        if (touchpadReport) {
            s_rawInputSeen = true;
            m_classifier.contactReport(time);
            // Log the start of each burst of reports, then every 20th.
            m_inputBurst =
              quint32(time - m_lastInputTime) > 300 ? 0 : m_inputBurst + 1;
            m_lastInputTime = time;
            if (InputTrace::enabled() &&
                (m_inputBurst < 5 || m_inputBurst % 20 == 0)) {
                InputTrace::write(
                  QStringLiteral("input time=%1 sink=%2 burst=%3 test=%4")
                    .arg(time)
                    .arg(msg->message == WM_INPUT
                           ? static_cast<int>(
                               GET_RAWINPUT_CODE_WPARAM(msg->wParam))
                           : -1)
                    .arg(m_inputBurst)
                    .arg(msg->message != WM_INPUT));
            }
        }
    } else if (msg->message == WM_MOUSEWHEEL ||
               msg->message == WM_MOUSEHWHEEL) {
        INPUT_MESSAGE_SOURCE source{};
        const bool sourceKnown = GetCurrentInputMessageSource(&source);
        const bool sourceIsTouchpad =
          sourceKnown && source.deviceType == IMDT_TOUCHPAD;
        const int delta = GET_WHEEL_DELTA_WPARAM(msg->wParam);
        if (msg->message == WM_MOUSEWHEEL) {
            const auto reason =
              m_classifier.classifyWheel(time, delta, sourceIsTouchpad);
            m_fromTouchpad = reason != TouchpadScrollClassifier::Reason::Mouse;
            m_naturalScroll = m_fromTouchpad && m_naturalSetting;
            if (InputTrace::enabled()) {
                InputTrace::write(
                  QStringLiteral("wheel time=%1 delta=%2 extra=0x%3 "
                                 "device=0x%4 origin=0x%5 reason=%6 "
                                 "natural=%7 foreground=%8")
                    .arg(time)
                    .arg(delta)
                    .arg(static_cast<quintptr>(GetMessageExtraInfo()), 0, 16)
                    .arg(sourceKnown ? int(source.deviceType) : -1, 0, 16)
                    .arg(sourceKnown ? int(source.originId) : -1, 0, 16)
                    .arg(reasonName(reason))
                    .arg(m_naturalScroll)
                    .arg(GetForegroundWindow() == msg->hwnd));
            }
        } else if (InputTrace::enabled()) {
            InputTrace::write(
              QStringLiteral("hwheel time=%1 delta=%2").arg(time).arg(delta));
        }
    }
    return msg->message == WM_ACTIVATEAPP || msg->message == WM_NCACTIVATE;
}

bool nativeSmartRegionDetectorSupported()
{
    return true;
}

std::unique_ptr<SmartRegionDetector> createNativeSmartRegionDetector()
{
    return std::make_unique<WindowsSmartRegionDetector>();
}
