// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "globalshortcutfilter.h"
#include "core/flameshot.h"
#include "utils/confighandler.h"

#include <QKeySequence>

#include <qt_windows.h>

GlobalShortcutFilter::GlobalShortcutFilter(QObject* parent)
  : QObject(parent)
{
    QObject::connect(
      ConfigHandler::getInstance(),
      &ConfigHandler::shortcutChanged,
      this,
      [this](const QString& actionName, const QString& shortcut) {
          if (actionName == QStringLiteral("TAKE_SCREENSHOT")) {
              reloadPrintScreenShortcut(shortcut);
          }
      });
    QObject::connect(ConfigHandler::getInstance(),
                     &ConfigHandler::fileChanged,
                     this,
                     [this]() {
                         reloadPrintScreenShortcut(
                           ConfigHandler().shortcut("TAKE_SCREENSHOT"));
                     });

    reloadPrintScreenShortcut(ConfigHandler().shortcut("TAKE_SCREENSHOT"));

#ifdef ENABLE_IMGUR
    m_historyShortcutRegistered =
      RegisterHotKey(NULL, 2, MOD_SHIFT, VK_SNAPSHOT) != FALSE;
#endif
}

GlobalShortcutFilter::~GlobalShortcutFilter()
{
    if (m_printScreenRegistered) {
        UnregisterHotKey(NULL, 1);
    }
#ifdef ENABLE_IMGUR
    if (m_historyShortcutRegistered) {
        UnregisterHotKey(NULL, 2);
    }
#endif
}

void GlobalShortcutFilter::reloadPrintScreenShortcut(const QString& shortcut)
{
    const bool shouldRegister =
      QKeySequence(shortcut) == QKeySequence(Qt::Key_Print);

    if (!shouldRegister) {
        if (m_printScreenRegistered) {
            UnregisterHotKey(NULL, 1);
            m_printScreenRegistered = false;
        }
        return;
    }

    if (!m_printScreenRegistered) {
        m_printScreenRegistered =
          RegisterHotKey(NULL, 1, 0, VK_SNAPSHOT) != FALSE;
    }
}

bool GlobalShortcutFilter::nativeEventFilter(const QByteArray& eventType,
                                             void* message,
                                             qintptr* result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) {
        const quint32 keycode = HIWORD(msg->lParam);
        const quint32 modifiers = LOWORD(msg->lParam);
#ifdef ENABLE_IMGUR
        // Show screenshots history
        if (VK_SNAPSHOT == keycode && MOD_SHIFT == modifiers) {
            Flameshot::instance()->history();
            return true;
        }
#endif
        // Capture screen only when plain Print Screen is the configured
        // TAKE_SCREENSHOT shortcut and native registration succeeded.
        if (m_printScreenRegistered && VK_SNAPSHOT == keycode &&
            0 == modifiers) {
            Flameshot::instance()->requestCapture(
              CaptureRequest(CaptureRequest::GRAPHICAL_MODE));
            return true;
        }
    }
    return false; // Forward event to Qt
}
