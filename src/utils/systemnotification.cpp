#include "systemnotification.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QUrl>

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#endif

SystemNotification::SystemNotification(QObject* parent)
  : QObject(parent)
  , m_interface(nullptr)
{
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    m_interface =
      new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                         QStringLiteral("/org/freedesktop/Notifications"),
                         QStringLiteral("org.freedesktop.Notifications"),
                         QDBusConnection::sessionBus(),
                         this);
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
    Controller::getInstance()->sendTrayNotification(text, title, timeout);
#else
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
         << "flameshot"                  // icon
         << title                        // summary
         << text                         // body
         << QStringList()                // actions
         << hintsMap                     // hints
         << timeout;                     // timeout
    m_interface->callWithArgumentList(
      QDBus::AutoDetect, QStringLiteral("Notify"), args);
#endif
}
