// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

// X11 backend: the window manager's EWMH properties list the windows, and
// AT-SPI provides the UI elements inside them. Wayland does not expose other
// clients' windows, so it has no backend.

#include "smartregiondetector.h"
#include "smartregiongeometry.h"
#include "utils/desktopinfo.h"

#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QHash>
#include <QScreen>
#include <QSet>

#if QT_CONFIG(xcb)
#include <xcb/xcb.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace {
using SmartRegionGeometry::ScreenMapping;
using SmartRegionGeometry::toLogicalRect;

constexpr qint64 LOOKUP_BUDGET_MS = 200;
constexpr int MESSAGE_TIMEOUT_MS = 80;
constexpr int MAX_DEPTH = 32;
constexpr int MIN_WINDOW_SIDE = 20;

xcb_connection_t* x11Connection()
{
    auto* x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11 ? x11->connection() : nullptr;
}

template<typename T>
using XcbReply = std::unique_ptr<T, decltype(&std::free)>;

template<typename T>
XcbReply<T> wrap(T* reply)
{
    return XcbReply<T>(reply, &std::free);
}

enum Atom
{
    NET_CLIENT_LIST_STACKING,
    NET_FRAME_EXTENTS,
    GTK_FRAME_EXTENTS,
    NET_WM_PID,
    NET_WM_STATE,
    NET_WM_STATE_HIDDEN,
    NET_WM_WINDOW_TYPE,
    NET_WM_WINDOW_TYPE_DESKTOP,
    ATOM_COUNT
};

constexpr std::array<const char*, ATOM_COUNT> ATOM_NAMES = {
    "_NET_CLIENT_LIST_STACKING",
    "_NET_FRAME_EXTENTS",
    "_GTK_FRAME_EXTENTS",
    "_NET_WM_PID",
    "_NET_WM_STATE",
    "_NET_WM_STATE_HIDDEN",
    "_NET_WM_WINDOW_TYPE",
    "_NET_WM_WINDOW_TYPE_DESKTOP",
};

xcb_get_property_cookie_t requestProperty(xcb_connection_t* connection,
                                          xcb_window_t window,
                                          xcb_atom_t property,
                                          xcb_atom_t type,
                                          uint32_t length)
{
    return xcb_get_property(connection, 0, window, property, type, 0, length);
}

// Reads a property as a list of 32-bit values.
template<typename T>
QList<T> propertyValues(xcb_connection_t* connection,
                        xcb_get_property_cookie_t cookie)
{
    static_assert(sizeof(T) == 4);
    const auto reply =
      wrap(xcb_get_property_reply(connection, cookie, nullptr));
    if (!reply || reply->format != 32) {
        return {};
    }
    const auto* values =
      static_cast<const T*>(xcb_get_property_value(reply.get()));
    const int count = xcb_get_property_value_length(reply.get()) / 4;
    return QList<T>(values, values + count);
}

// Left, right, top and bottom extents, as stored in _NET_FRAME_EXTENTS.
QMargins frameMargins(const QList<uint32_t>& values)
{
    if (values.size() != 4) {
        return {};
    }
    return QMargins(static_cast<int>(values.at(0)),
                    static_cast<int>(values.at(2)),
                    static_cast<int>(values.at(1)),
                    static_cast<int>(values.at(3)));
}

// Synchronous AT-SPI client sharing one time budget per lookup.
class AtSpi
{
public:
    struct Reference
    {
        QString service;
        QString path;
        bool isNull() const
        {
            return service.isEmpty() ||
                   path == QLatin1String("/org/a11y/atspi/null");
        }
    };

    explicit AtSpi(QDBusConnection connection)
      : m_bus(std::move(connection))
    {}

    static std::unique_ptr<AtSpi> connect()
    {
        QString address = qEnvironmentVariable("AT_SPI_BUS_ADDRESS");
        if (address.isEmpty()) {
            const QDBusMessage reply = QDBusConnection::sessionBus().call(
              QDBusMessage::createMethodCall(QStringLiteral("org.a11y.Bus"),
                                             QStringLiteral("/org/a11y/bus"),
                                             QStringLiteral("org.a11y.Bus"),
                                             QStringLiteral("GetAddress")),
              QDBus::Block,
              MESSAGE_TIMEOUT_MS);
            if (reply.type() == QDBusMessage::ReplyMessage &&
                !reply.arguments().isEmpty()) {
                address = reply.arguments().constFirst().toString();
            }
        }
        if (address.isEmpty()) {
            return nullptr;
        }
        // Connect afresh for each capture: Qt reuses a named connection even
        // when it was lost or the accessibility bus has since restarted.
        const QString name = QStringLiteral("flameshot-smart-region-atspi");
        QDBusConnection::disconnectFromBus(name);
        QDBusConnection bus = QDBusConnection::connectToBus(address, name);
        if (!bus.isConnected()) {
            return nullptr;
        }
        return std::make_unique<AtSpi>(bus);
    }

    void startLookup()
    {
        m_elapsed.start();
        m_failed = false;
    }

    bool exhausted() const
    {
        return m_failed || m_elapsed.elapsed() >= LOOKUP_BUDGET_MS;
    }

    // The application owning the given process, if it is registered. The
    // applications are listed once per capture, with a budget of their own.
    Reference application(uint pid)
    {
        if (!m_applicationsListed) {
            m_applicationsListed = true;
            startLookup();
            const Reference root{ QStringLiteral("org.a11y.atspi.Registry"),
                                  QStringLiteral(
                                    "/org/a11y/atspi/accessible/root") };
            for (const Reference& app : children(root)) {
                if (const QVariant id =
                      callOne(QStringLiteral("org.freedesktop.DBus"),
                              QStringLiteral("/org/freedesktop/DBus"),
                              QStringLiteral("org.freedesktop.DBus"),
                              QStringLiteral("GetConnectionUnixProcessID"),
                              { app.service });
                    id.isValid()) {
                    m_applications.insert(id.toUInt(), app);
                }
            }
            startLookup();
        }
        return m_applications.value(pid);
    }

    QList<Reference> children(const Reference& object)
    {
        QList<Reference> result;
        QVariant reply = callOne(object.service,
                                 object.path,
                                 QStringLiteral("org.a11y.atspi.Accessible"),
                                 QStringLiteral("GetChildren"),
                                 {});
        if (!reply.canConvert<QDBusArgument>()) {
            return result;
        }
        const auto argument = reply.value<QDBusArgument>();
        argument.beginArray();
        while (!argument.atEnd()) {
            result.append(readReference(argument));
        }
        argument.endArray();
        return result;
    }

    // Screen coordinates of an element, or a null rect.
    QRect extents(const Reference& object)
    {
        const QVariant reply =
          callOne(object.service,
                  object.path,
                  QStringLiteral("org.a11y.atspi.Component"),
                  QStringLiteral("GetExtents"),
                  { QVariant::fromValue(SCREEN_COORDINATES) });
        if (!reply.canConvert<QDBusArgument>()) {
            return {};
        }
        int x = 0, y = 0, width = 0, height = 0;
        const auto argument = reply.value<QDBusArgument>();
        argument.beginStructure();
        argument >> x >> y >> width >> height;
        argument.endStructure();
        return { x, y, width, height };
    }

    Reference childAt(const Reference& object, const QPoint& position)
    {
        const QVariant reply =
          callOne(object.service,
                  object.path,
                  QStringLiteral("org.a11y.atspi.Component"),
                  QStringLiteral("GetAccessibleAtPoint"),
                  { position.x(),
                    position.y(),
                    QVariant::fromValue(SCREEN_COORDINATES) });
        if (!reply.canConvert<QDBusArgument>()) {
            return {};
        }
        return readReference(reply.value<QDBusArgument>());
    }

private:
    static constexpr uint SCREEN_COORDINATES = 0;

    static Reference readReference(const QDBusArgument& argument)
    {
        Reference reference;
        QDBusObjectPath path;
        argument.beginStructure();
        argument >> reference.service >> path;
        argument.endStructure();
        reference.path = path.path();
        return reference;
    }

    QVariant callOne(const QString& service,
                     const QString& path,
                     const QString& interface,
                     const QString& method,
                     const QVariantList& arguments)
    {
        const qint64 remaining = LOOKUP_BUDGET_MS - m_elapsed.elapsed();
        if (m_failed || remaining <= 0 || m_unresponsive.contains(service)) {
            return {};
        }
        QDBusMessage message =
          QDBusMessage::createMethodCall(service, path, interface, method);
        message.setArguments(arguments);
        const QDBusMessage reply = m_bus.call(
          message,
          QDBus::Block,
          static_cast<int>(std::min<qint64>(remaining, MESSAGE_TIMEOUT_MS)));
        if (reply.type() != QDBusMessage::ReplyMessage ||
            reply.arguments().isEmpty()) {
            // A timeout means the application is busy. Stop this lookup, and
            // do not ask that application again during this capture.
            if (reply.errorName() ==
                QLatin1String("org.freedesktop.DBus.Error.NoReply")) {
                m_failed = true;
                m_unresponsive.insert(service);
            }
            return {};
        }
        return reply.arguments().constFirst();
    }

    QDBusConnection m_bus;
    QElapsedTimer m_elapsed;
    bool m_failed = false;
    bool m_applicationsListed = false;
    QHash<uint, Reference> m_applications;
    QSet<QString> m_unresponsive;
};

class X11SmartRegionDetector : public SmartRegionDetector
{
public:
    void snapshotWindows() override;
    QList<QRect> regionsAt(const QPoint& globalPosition) override;
    bool elementDetectionAvailable() const override
    {
        return m_atSpi != nullptr;
    }

private:
    struct Window
    {
        QRect nativeGeometry;
        uint pid;
    };

    QList<QRect> elementsAt(const Window& window, const QPoint& nativePoint);

    // Front to back.
    QList<Window> m_windows;
    QList<ScreenMapping> m_screens;
    std::unique_ptr<AtSpi> m_atSpi;
};

void X11SmartRegionDetector::snapshotWindows()
{
    m_windows.clear();
    m_screens.clear();
    m_atSpi = AtSpi::connect();

    // Qt keeps each screen's origin in device pixels and scales its size.
    for (const QScreen* screen : QGuiApplication::screens()) {
        const QRect logical = screen->geometry();
        const QSize native =
          (QSizeF(logical.size()) * screen->devicePixelRatio()).toSize();
        m_screens.append({ QRect(logical.topLeft(), native), logical });
    }

    xcb_connection_t* connection = x11Connection();
    if (!connection) {
        return;
    }
    const xcb_window_t root =
      xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;

    std::array<xcb_intern_atom_cookie_t, ATOM_COUNT> atomCookies{};
    for (int i = 0; i < ATOM_COUNT; ++i) {
        atomCookies[i] = xcb_intern_atom(
          connection, 1, std::strlen(ATOM_NAMES[i]), ATOM_NAMES[i]);
    }
    std::array<xcb_atom_t, ATOM_COUNT> atoms{};
    for (int i = 0; i < ATOM_COUNT; ++i) {
        const auto reply =
          wrap(xcb_intern_atom_reply(connection, atomCookies[i], nullptr));
        atoms[i] = reply ? reply->atom : XCB_ATOM_NONE;
    }

    // Bottom to top, per EWMH.
    const QList<xcb_window_t> clients = propertyValues<xcb_window_t>(
      connection,
      requestProperty(connection,
                      root,
                      atoms[NET_CLIENT_LIST_STACKING],
                      XCB_ATOM_WINDOW,
                      UINT32_MAX / 4));

    // Send every request first, then read the replies, so the window manager
    // and X server are asked in a single round trip.
    struct Requests
    {
        xcb_get_window_attributes_cookie_t attributes;
        xcb_get_geometry_cookie_t geometry;
        xcb_translate_coordinates_cookie_t position;
        xcb_get_property_cookie_t frameExtents;
        xcb_get_property_cookie_t clientSideExtents;
        xcb_get_property_cookie_t pid;
        xcb_get_property_cookie_t state;
        xcb_get_property_cookie_t type;
    };
    QList<Requests> requests;
    requests.reserve(clients.size());
    for (const xcb_window_t client : clients) {
        requests.append(
          { xcb_get_window_attributes(connection, client),
            xcb_get_geometry(connection, client),
            xcb_translate_coordinates(connection, client, root, 0, 0),
            requestProperty(connection,
                            client,
                            atoms[NET_FRAME_EXTENTS],
                            XCB_ATOM_CARDINAL,
                            4),
            requestProperty(connection,
                            client,
                            atoms[GTK_FRAME_EXTENTS],
                            XCB_ATOM_CARDINAL,
                            4),
            requestProperty(
              connection, client, atoms[NET_WM_PID], XCB_ATOM_CARDINAL, 1),
            requestProperty(
              connection, client, atoms[NET_WM_STATE], XCB_ATOM_ATOM, 32),
            requestProperty(connection,
                            client,
                            atoms[NET_WM_WINDOW_TYPE],
                            XCB_ATOM_ATOM,
                            32) });
    }

    const auto ownPid = static_cast<uint>(QCoreApplication::applicationPid());
    for (auto request = requests.crbegin(); request != requests.crend();
         ++request) {
        const auto attributes = wrap(xcb_get_window_attributes_reply(
          connection, request->attributes, nullptr));
        const auto geometry =
          wrap(xcb_get_geometry_reply(connection, request->geometry, nullptr));
        const auto position = wrap(xcb_translate_coordinates_reply(
          connection, request->position, nullptr));
        const QList<uint32_t> pid =
          propertyValues<uint32_t>(connection, request->pid);
        const QList<xcb_atom_t> state =
          propertyValues<xcb_atom_t>(connection, request->state);
        const QList<xcb_atom_t> type =
          propertyValues<xcb_atom_t>(connection, request->type);
        // The decorations drawn by the window manager belong to the window,
        // while the shadows of client-side decorations do not.
        const QMargins frame = frameMargins(
          propertyValues<uint32_t>(connection, request->frameExtents));
        const QMargins shadow = frameMargins(
          propertyValues<uint32_t>(connection, request->clientSideExtents));

        if (!attributes || !geometry || !position ||
            attributes->map_state != XCB_MAP_STATE_VIEWABLE ||
            state.contains(atoms[NET_WM_STATE_HIDDEN]) ||
            type.contains(atoms[NET_WM_WINDOW_TYPE_DESKTOP]) ||
            (!pid.isEmpty() && pid.constFirst() == ownPid)) {
            continue;
        }
        const QRect bounds =
          QRect(
            position->dst_x, position->dst_y, geometry->width, geometry->height)
            .marginsAdded(frame)
            .marginsRemoved(shadow);
        if (bounds.width() < MIN_WINDOW_SIDE ||
            bounds.height() < MIN_WINDOW_SIDE) {
            continue;
        }
        m_windows.append({ bounds, pid.isEmpty() ? 0 : pid.constFirst() });
    }
}

QList<QRect> X11SmartRegionDetector::regionsAt(const QPoint& globalPosition)
{
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
    for (const QRect& element : elementsAt(*window, nativePoint)) {
        elements.append(toLogicalRect(element, *screen));
    }
    return buildRegionChain(
      elements, toLogicalRect(window->nativeGeometry, *screen), globalPosition);
}

QList<QRect> X11SmartRegionDetector::elementsAt(const Window& window,
                                                const QPoint& nativePoint)
{
    if (!m_atSpi || window.pid == 0) {
        return {};
    }
    m_atSpi->startLookup();
    const AtSpi::Reference application = m_atSpi->application(window.pid);
    if (application.isNull()) {
        return {};
    }

    // Pick the application's top-level element that matches the window best
    // (a dialog and its main window can both contain the pointer), then
    // descend to the deepest element under the pointer.
    const auto area = [](const QRect& rect) {
        return rect.isEmpty() ? 0.0 : double(rect.width()) * rect.height();
    };
    AtSpi::Reference current;
    QRect geometry;
    double bestMatch = 0;
    for (const AtSpi::Reference& frame : m_atSpi->children(application)) {
        const QRect frameGeometry = m_atSpi->extents(frame);
        if (!frameGeometry.contains(nativePoint)) {
            continue;
        }
        const double overlap =
          area(frameGeometry.intersected(window.nativeGeometry));
        const double match = overlap / (area(frameGeometry) +
                                        area(window.nativeGeometry) - overlap);
        if (match > bestMatch) {
            bestMatch = match;
            current = frame;
            geometry = frameGeometry;
        }
    }

    QList<QRect> elements;
    for (int depth = 0; !current.isNull() && depth < MAX_DEPTH; ++depth) {
        if (depth > 0) {
            geometry = m_atSpi->extents(current);
        }
        if (!geometry.contains(nativePoint)) {
            break;
        }
        elements.prepend(geometry);
        const AtSpi::Reference child = m_atSpi->childAt(current, nativePoint);
        if (child.service == current.service && child.path == current.path) {
            break;
        }
        current = child;
    }
    return m_atSpi->exhausted() ? QList<QRect>() : elements;
}
} // namespace

bool nativeSmartRegionDetectorSupported()
{
    // XWayland only sees X11 clients, so window lists would be incomplete.
    return QGuiApplication::platformName() == QLatin1String("xcb") &&
           !DesktopInfo().waylandDetected() && x11Connection();
}

std::unique_ptr<SmartRegionDetector> createNativeSmartRegionDetector()
{
    return std::make_unique<X11SmartRegionDetector>();
}

#else // QT_CONFIG(xcb)

bool nativeSmartRegionDetectorSupported()
{
    return false;
}

std::unique_ptr<SmartRegionDetector> createNativeSmartRegionDetector()
{
    return nullptr;
}

#endif
