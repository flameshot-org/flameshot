#include "systemnotification.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QUrl>

#ifndef Q_OS_WIN
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#else
#endif
#include "src/core/controller.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}
#else
SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = nullptr;
}
#endif

void SystemNotification::sendMessage(const QString &text, const QString &savePath) {
    sendMessage(text, tr("Flameshot Info"), savePath);
}

void SystemNotification::sendMessage(
        const QString &text,
        const QString &title,
        const QString &savePath,
        const int timeout)
{
    if(!ConfigHandler().desktopNotificationValue()) {
        return;
    }

#ifndef Q_OS_WIN
    QList<QVariant> args;
    QVariantMap hintsMap;
    if (!savePath.isEmpty()) {
        QUrl fullPath = QUrl::fromLocalFile(savePath);
        // allows the notification to be dragged and dropped
        hintsMap[QStringLiteral("x-kde-urls")] = QStringList({fullPath.toString()});
    }
    args << (qAppName())                 //appname
         << static_cast<unsigned int>(0) //id
         << "flameshot"                  //icon
         << title                        //summary
         << text                         //body
         << QStringList()                //actions
         << hintsMap                     //hints
         << timeout;                     //timeout
    m_interface->callWithArgumentList(QDBus::AutoDetect, QStringLiteral("Notify"), args);
#else
    auto c = Controller::getInstance();
    c->sendTrayNotification(text, title, timeout);
#endif
}
