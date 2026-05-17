#include "systemnotification.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
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
  , m_interface(nullptr)
{
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    if (!ConfigHandler().showDesktopNotification()) {
        return;
    }
    auto bus = QDBusConnection::sessionBus();
    auto* connectionInterface = bus.interface();

    auto service = QStringLiteral("org.freedesktop.Notifications");
    auto path = QStringLiteral("/org/freedesktop/Notifications");
    auto interface = QStringLiteral("org.freedesktop.Notifications");

    if (connectionInterface->isServiceRegistered(service)) {
        m_interface = new QDBusInterface(service, path, interface, bus, this);
    } else {
        AbstractLogger::warning(AbstractLogger::Stderr |
                                AbstractLogger::LogFile)
          << tr("No DBus System Notification service found");
    }
#endif
}

QMap<uint, QString> SystemNotification::s_pendingPaths;

SystemNotification* SystemNotification::actionHandler()
{
    static SystemNotification* handler = [] {
        auto* h = new SystemNotification(qApp);
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
        QDBusConnection::sessionBus().connect(
          QStringLiteral("org.freedesktop.Notifications"),
          QStringLiteral("/org/freedesktop/Notifications"),
          QStringLiteral("org.freedesktop.Notifications"),
          QStringLiteral("ActionInvoked"),
          h,
          SLOT(onActionInvoked(uint, QString)));
#endif
        return h;
    }();
    return handler;
}

void SystemNotification::onActionInvoked(uint id, const QString& actionKey)
{
    if (actionKey == QLatin1String("default")) {
        auto it = s_pendingPaths.find(id);
        if (it != s_pendingPaths.end()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(it.value()));
            s_pendingPaths.erase(it);
            if (s_pendingPaths.isEmpty()) {
                qApp->exit();
            }
        }
    }
}

bool SystemNotification::hasPendingPaths()
{
    return !s_pendingPaths.isEmpty();
}

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
      this,
      [&]() {
          // The call is queued to avoid recursive static initialization of
          // Flameshot and ConfigHandler.
          if (FlameshotDaemon::instance())
              FlameshotDaemon::instance()->sendTrayNotification(
                text, title, timeout);
      },
      Qt::QueuedConnection);
#else
    if (nullptr != m_interface && m_interface->isValid()) {
        QList<QVariant> args;
        QVariantMap hintsMap;
        QStringList actions;
        if (!savePath.isEmpty()) {
            QUrl fullPath = QUrl::fromLocalFile(savePath);
            // allows the notification to be dragged and dropped
            hintsMap[QStringLiteral("x-kde-urls")] =
              QStringList({ fullPath.toString() });
            // makes the notification body clickable (freedesktop spec)
            actions << QStringLiteral("default") << QString();
        }

        args << (qAppName())                 // appname
             << static_cast<unsigned int>(0) // id
             << FLAMESHOT_ICON               // icon
             << title                        // summary
             << text                         // body
             << actions                      // actions
             << hintsMap                     // hints
             << timeout;                     // timeout

        QDBusReply<uint> reply = m_interface->callWithArgumentList(
          QDBus::AutoDetect, QStringLiteral("Notify"), args);

        if (reply.isValid() && !savePath.isEmpty()) {
            actionHandler();
            s_pendingPaths[reply.value()] = savePath;
        }
    }
#endif
}
