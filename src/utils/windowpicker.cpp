// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "windowpicker.h"

#include <QGuiApplication>
#include <QScreen>

#if defined(Q_OS_WIN)
#include <dwmapi.h>
#include <windows.h>
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <xcb/xcb.h>

#include <unistd.h>

#include <cstring>
#endif

namespace {

#if defined(Q_OS_WIN)

/**
 * The DWM reports the visible frame (without the invisible resize border and
 * drop shadow) through DWMWA_EXTENDED_FRAME_BOUNDS; fall back to the full
 * window rectangle when DWM is not running.
 */
RECT frameBounds(HWND hwnd)
{
    RECT rect{};
    if (FAILED(DwmGetWindowAttribute(
          hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect)))) {
        if (!GetWindowRect(hwnd, &rect)) {
            return RECT{};
        }
    }
    return rect;
}

bool isCloaked(HWND hwnd)
{
    DWORD cloaked = 0;
    if (FAILED(DwmGetWindowAttribute(
          hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked)))) {
        return false;
    }
    return cloaked != 0;
}

bool isOwnWindow(HWND hwnd)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid == GetCurrentProcessId();
}

bool isDesktopWindow(HWND hwnd)
{
    wchar_t name[64] = {};
    if (GetClassNameW(hwnd, name, 64) == 0) {
        return false;
    }
    return wcscmp(name, L"Progman") == 0 || wcscmp(name, L"WorkerW") == 0;
}

struct PickState
{
    POINT point;
    HWND found;
};

BOOL CALLBACK pickWindowProc(HWND hwnd, LPARAM lparam)
{
    auto* state = reinterpret_cast<PickState*>(lparam);
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) {
        return TRUE;
    }
    if (isOwnWindow(hwnd) || isDesktopWindow(hwnd) || isCloaked(hwnd)) {
        return TRUE;
    }
    const RECT rect = frameBounds(hwnd);
    if (rect.right <= rect.left || rect.bottom <= rect.top) {
        return TRUE;
    }
    if (!PtInRect(&rect, state->point)) {
        return TRUE;
    }
    // EnumWindows walks top-level windows in front-to-back order
    state->found = hwnd;
    return FALSE;
}

#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)

bool isOwnWindow(xcb_connection_t* connection,
                 xcb_window_t window,
                 xcb_atom_t pidAtom)
{
    if (pidAtom == XCB_ATOM_NONE) {
        return false;
    }
    xcb_get_property_reply_t* reply = xcb_get_property_reply(
      connection,
      xcb_get_property(connection, 0, window, pidAtom, XCB_ATOM_CARDINAL, 0, 1),
      nullptr);
    if (reply == nullptr) {
        return false;
    }
    bool own = false;
    if (reply->format == 32 && xcb_get_property_value_length(reply) >= 4) {
        const auto pid =
          *reinterpret_cast<const uint32_t*>(xcb_get_property_value(reply));
        own = pid == static_cast<uint32_t>(getpid());
    }
    free(reply);
    return own;
}

struct WindowRect
{
    bool valid = false;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

/**
 * Top-most viewable top-level window (direct child of the root window)
 * containing the point, in root (physical pixel) coordinates. Windows owned by
 * this process are skipped so the capture overlay does not shadow what is
 * below it.
 */
WindowRect topLevelWindowAt(int x, int y)
{
    WindowRect result;
    xcb_connection_t* connection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(connection)) {
        xcb_disconnect(connection);
        return result;
    }
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t screens = xcb_setup_roots_iterator(setup);
    if (screens.rem == 0) {
        xcb_disconnect(connection);
        return result;
    }
    const xcb_window_t root = screens.data->root;

    static constexpr char PID_ATOM[] = "_NET_WM_PID";
    xcb_intern_atom_reply_t* atomReply = xcb_intern_atom_reply(
      connection,
      xcb_intern_atom(connection, 0, std::strlen(PID_ATOM), PID_ATOM),
      nullptr);
    const xcb_atom_t pidAtom = atomReply ? atomReply->atom : XCB_ATOM_NONE;
    free(atomReply);

    xcb_query_tree_reply_t* tree = xcb_query_tree_reply(
      connection, xcb_query_tree(connection, root), nullptr);
    if (tree != nullptr) {
        const xcb_window_t* children = xcb_query_tree_children(tree);
        const int count = xcb_query_tree_children_length(tree);
        // children are listed bottom-most first, so walk back to front
        for (int i = count - 1; i >= 0; --i) {
            const xcb_window_t candidate = children[i];
            xcb_get_window_attributes_reply_t* attrs =
              xcb_get_window_attributes_reply(
                connection,
                xcb_get_window_attributes(connection, candidate),
                nullptr);
            if (attrs == nullptr) {
                continue;
            }
            const bool viewable = attrs->map_state == XCB_MAP_STATE_VIEWABLE &&
                                  attrs->_class != XCB_WINDOW_CLASS_INPUT_ONLY;
            free(attrs);
            if (!viewable) {
                continue;
            }
            xcb_get_geometry_reply_t* geometry = xcb_get_geometry_reply(
              connection, xcb_get_geometry(connection, candidate), nullptr);
            if (geometry == nullptr) {
                continue;
            }
            const bool contains = x >= geometry->x && y >= geometry->y &&
                                  x < geometry->x + geometry->width &&
                                  y < geometry->y + geometry->height;
            const bool usable = geometry->width > 0 && geometry->height > 0;
            if (contains && usable &&
                !isOwnWindow(connection, candidate, pidAtom)) {
                result.valid = true;
                result.x = geometry->x;
                result.y = geometry->y;
                result.width = geometry->width;
                result.height = geometry->height;
                free(geometry);
                break;
            }
            free(geometry);
        }
        free(tree);
    }
    xcb_disconnect(connection);
    return result;
}

#endif

} // namespace

QRect WindowPicker::windowRectAt(const QPoint& globalPos)
{
#if defined(Q_OS_WIN)
    QScreen* screen = QGuiApplication::screenAt(globalPos);
    const qreal dpr = screen ? screen->devicePixelRatio() : 1.0;
    PickState state{ POINT{ qRound(globalPos.x() * dpr),
                            qRound(globalPos.y() * dpr) },
                     nullptr };
    EnumWindows(&pickWindowProc, reinterpret_cast<LPARAM>(&state));
    if (state.found == nullptr) {
        return {};
    }
    const RECT rect = frameBounds(state.found);
    if (rect.right <= rect.left || rect.bottom <= rect.top) {
        return {};
    }
    return QRect(qRound(rect.left / dpr),
                 qRound(rect.top / dpr),
                 qRound((rect.right - rect.left) / dpr),
                 qRound((rect.bottom - rect.top) / dpr));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        return {};
    }
    QScreen* screen = QGuiApplication::screenAt(globalPos);
    const qreal dpr = screen ? screen->devicePixelRatio() : 1.0;
    const WindowRect window = topLevelWindowAt(qRound(globalPos.x() * dpr),
                                               qRound(globalPos.y() * dpr));
    if (!window.valid) {
        return {};
    }
    return QRect(qRound(window.x / dpr),
                 qRound(window.y / dpr),
                 qRound(window.width / dpr),
                 qRound(window.height / dpr));
#else
    Q_UNUSED(globalPos)
    return {};
#endif
}
