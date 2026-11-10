// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "systemnotification.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"

#include <QApplication>
#include <QFileInfo>
#include <QUrl>
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDesktopServices>
#else
#include "core/flameshotdaemon.h"
#endif

// work-around for snap, which cannot install icons into
// the system folder, so instead the absolute path to the
// icon (saved somewhere in /snap/flameshot/...) is passed
#ifndef FLAMESHOT_ICON
#define FLAMESHOT_ICON "flameshot"
#endif

SystemNotification::SystemNotification(QObject* parent)
  : QObject(parent)
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
  , m_interface(nullptr)
#endif
{}

void SystemNotification::sendMessage(const QString& text,
                                     const QString& savePath)
{
    sendMessage(text, tr("Flameshot Info"), savePath);
}

void SystemNotification::sendMessage(const QString& text,
                                     const QString& title,
                                     const QString& savePath,
                                     const int timeout)
{
    if (!ConfigHandler().showDesktopNotification()) {
        return;
    }

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QMetaObject::invokeMethod(
      qApp,
      [text, title, timeout, savePath]() {
          // The call is queued to avoid recursive static initialization of
          // Flameshot and ConfigHandler.
          if (FlameshotDaemon::instance()) {
              FlameshotDaemon::instance()->sendTrayNotification(
                text, title, timeout, savePath);
          }
      },
      Qt::QueuedConnection);
#else
    ensureInterface();
    if (m_interface == nullptr) {
        return;
    }

    QVariantMap hintsMap;
    if (!savePath.isEmpty()) {
        QUrl fullPath = QUrl::fromLocalFile(savePath);
        // allows the notification to be dragged and dropped
        hintsMap[QStringLiteral("x-kde-urls")] =
          QStringList({ fullPath.toString() });
    }

    // "default" is invoked by the notification server when the body of the
    // notification is clicked; "open-folder" is an explicit button for servers
    // that do not expose the default action as a button.
    QStringList actions;
    if (!savePath.isEmpty() && m_supportsActions) {
        actions << QStringLiteral("default") << tr("Open folder")
                << QStringLiteral("open-folder") << tr("Open folder");
    }

    QList<QVariant> args;
    args << (qAppName())                 // appname
         << static_cast<unsigned int>(0) // id
         << FLAMESHOT_ICON               // icon
         << title                        // summary
         << text                         // body
         << actions                      // actions
         << hintsMap                     // hints
         << timeout;                     // timeout

    // Fire-and-forget: an asynchronous call never blocks the event loop, even
    // when no notification daemon is registered on the session bus (e.g. bare
    // startx / tiling WM sessions). The previous synchronous
    // callWithArgumentList stalled the main thread for the QtDBus reply timeout
    // (~25s) after every capture, freezing further captures until it returned.
    QDBusPendingCall pending =
      m_interface->asyncCallWithArgumentList(QStringLiteral("Notify"), args);

    if (!savePath.isEmpty() && m_supportsActions) {
        const QString folder = QFileInfo(savePath).absolutePath();
        auto* watcher = new QDBusPendingCallWatcher(pending, this);
        connect(watcher,
                &QDBusPendingCallWatcher::finished,
                this,
                [this, folder](QDBusPendingCallWatcher* w) {
                    QDBusPendingReply<uint> reply = *w;
                    if (reply.isValid()) {
                        m_pendingFolders.insert(reply.value(), folder);
                        // Do not let the map grow without bound when several
                        // captures are saved in a row.
                        while (m_pendingFolders.size() > 32) {
                            m_pendingFolders.erase(m_pendingFolders.begin());
                        }
                    }
                    w->deleteLater();
                });
    }
#endif
}

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
void SystemNotification::ensureInterface()
{
    auto bus = QDBusConnection::sessionBus();
    auto* connectionInterface = bus.interface();

    const auto service = QStringLiteral("org.freedesktop.Notifications");
    const auto path = QStringLiteral("/org/freedesktop/Notifications");
    const auto interface = QStringLiteral("org.freedesktop.Notifications");

    if (connectionInterface == nullptr ||
        !connectionInterface->isServiceRegistered(service)) {
        delete m_interface;
        m_interface = nullptr;
        AbstractLogger::warning(AbstractLogger::Stderr |
                                AbstractLogger::LogFile)
          << tr("No DBus System Notification service found");
        return;
    }

    if (m_interface != nullptr && m_interface->isValid()) {
        return;
    }
    delete m_interface;
    m_interface = nullptr;

    if (!m_signalsConnected) {
        bus.connect(service,
                    path,
                    interface,
                    QStringLiteral("ActionInvoked"),
                    this,
                    SLOT(onActionInvoked(uint, QString)));
        bus.connect(service,
                    path,
                    interface,
                    QStringLiteral("NotificationClosed"),
                    this,
                    SLOT(onNotificationClosed(uint, uint)));
        m_signalsConnected = true;
    }

    m_interface = new QDBusInterface(service, path, interface, bus, this);
    if (!m_interface->isValid()) {
        delete m_interface;
        m_interface = nullptr;
        return;
    }

    queryCapabilities();
}

void SystemNotification::queryCapabilities()
{
    if (m_capabilitiesRequested || m_interface == nullptr) {
        return;
    }
    m_capabilitiesRequested = true;

    QDBusPendingCall pending =
      m_interface->asyncCall(QStringLiteral("GetCapabilities"));
    auto* watcher = new QDBusPendingCallWatcher(pending, this);
    connect(watcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this](QDBusPendingCallWatcher* w) {
                QDBusPendingReply<QStringList> reply = *w;
                // If the server does not answer, assume that it supports
                // actions: GNOME Shell, KDE Plasma, dunst and mako all do.
                if (reply.isValid()) {
                    m_supportsActions =
                      reply.value().contains(QStringLiteral("actions"));
                }
                w->deleteLater();
            });
}

void SystemNotification::onActionInvoked(uint id, const QString& actionKey)
{
    if (actionKey != QLatin1String("default") &&
        actionKey != QLatin1String("open-folder")) {
        return;
    }

    const QString folder = m_pendingFolders.take(id);
    if (folder.isEmpty() || !QFileInfo(folder).isDir()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
}

void SystemNotification::onNotificationClosed(uint id, uint /*reason*/)
{
    m_pendingFolders.remove(id);
}
#endif
