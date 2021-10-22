#include "flameshotdaemon.h"

#include "dbusutils.h"
#include "pinwidget.h"
#include "screenshotsaver.h"
#include <QApplication>
#include <QClipboard>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPixmap>
#include <QRect>

// TODO handle if daemon can't be contacted via dbus

void FlameshotDaemon::start()
{
    if (!m_instance) {
        m_instance = new FlameshotDaemon();
    }
}

void FlameshotDaemon::createPin(QPixmap capture, QRect geometry)
{
    if (instance()) {
        instance()->attachPin(capture, geometry);
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << capture;
    stream << geometry;
    QDBusMessage m =
      QDBusMessage::createMethodCall(QStringLiteral("org.flameshot.Flameshot"),
                                     QStringLiteral("/"),
                                     QLatin1String(""),
                                     QStringLiteral("attachPin"));
    m << data;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    DBusUtils().checkDBusConnection(sessionBus);
    sessionBus.call(m);
}

void FlameshotDaemon::copyToClipboard(QPixmap capture)
{
    if (instance()) {
        instance()->attachClipboard(capture);
        return;
    }
    QDBusMessage m =
      QDBusMessage::createMethodCall(QStringLiteral("org.flameshot.Flameshot"),
                                     QStringLiteral("/"),
                                     QLatin1String(""),
                                     QStringLiteral("attachClipboard"));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << capture;

    m << data;

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    DBusUtils().checkDBusConnection(sessionBus);
    sessionBus.call(m);
}

FlameshotDaemon::FlameshotDaemon()
  : m_persist(false)
  , m_hostingClipboard(false)
{
    // TODO on windows/(Mac maybe?), always persist so hotkeys can be used
    // TODO consider which config options could influence this
}

FlameshotDaemon* FlameshotDaemon::instance()
{
    return m_instance;
}

/**
 * @brief Quit the daemon if it has nothing to do and the 'persist' flag is not
 * set.
 */
void FlameshotDaemon::quitIfIdle()
{
    if (m_persist) {
        return;
    }
    if (!m_hostingClipboard && m_widgets.isEmpty()) {
        qApp->exit(0);
    }
}

// SERVICE METHODS

void FlameshotDaemon::attachPin(QPixmap pixmap, QRect geometry)
{
    PinWidget* pinWidget = new PinWidget(pixmap, geometry);
    connect(pinWidget, &QObject::destroyed, this, [=]() {
        m_widgets.removeOne(pinWidget);
        quitIfIdle();
    });

    pinWidget->show();
    pinWidget->activateWindow();
}

void FlameshotDaemon::attachClipboard(QPixmap pixmap)
{
    QClipboard* clipboard = QApplication::clipboard();
    connect(clipboard,
            &QClipboard::dataChanged,
            m_instance,
            &FlameshotDaemon::onClipboardChanged,
            Qt::UniqueConnection);

    clipboard->blockSignals(true);
    ScreenshotSaver().saveToClipboard(pixmap);
    clipboard->blockSignals(false);
}

// D-BUS ADAPTER METHODS

void FlameshotDaemon::attachPin(const QByteArray& data)
{
    QDataStream stream(data);
    QPixmap pixmap;
    QRect geometry;

    stream >> pixmap;
    stream >> geometry;

    attachPin(pixmap, geometry);
}

void FlameshotDaemon::attachClipboard(const QByteArray& screenshot)
{
    m_hostingClipboard = true;

    QDataStream stream(screenshot);
    QPixmap p;
    stream >> p;

    attachClipboard(p);
}

void FlameshotDaemon::onClipboardChanged()
{
    m_hostingClipboard = false;
    quitIfIdle();
}

// STATIC ATTRIBUTES
FlameshotDaemon* FlameshotDaemon::m_instance = nullptr;
