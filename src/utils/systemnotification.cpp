#include "systemnotification.h"
#include "src/core/flameshot.h"
#include "src/utils/abstractlogger.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QUrl>

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#else
#include "src/core/flameshotdaemon.h"
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
        if (!savePath.isEmpty()) {
            QUrl fullPath = QUrl::fromLocalFile(savePath);
            // allows the notification to be dragged and dropped
            hintsMap[QStringLiteral("x-kde-urls")] =
              QStringList({ fullPath.toString() });
        }

        args << (qAppName())                 // appname
             << static_cast<unsigned int>(0) // id
             << FLAMESHOT_ICON               // icon
             << title                        // summary
             << text                         // body
             << QStringList()                // actions
             << hintsMap                     // hints
             << timeout;                     // timeout
        m_interface->callWithArgumentList(
          QDBus::AutoDetect, QStringLiteral("Notify"), args);
    }
#endif
}
