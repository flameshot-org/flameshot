#include "flameshotdaemon.h"
#include "core/flameshot.h"
#include "tools/pin/pinwidget.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"
#include "utils/screenshotsaver.h"
#include "widgets/capture/capturewidget.h"
#include "widgets/trayicon.h"

#include <QApplication>
#include <QClipboard>
#include <QIODevice>
#include <QPixmap>
#include <QRect>
#include <memory>

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QDBusConnection>
#include <QDBusMessage>
#endif

#if !defined(DISABLE_UPDATE_CHECKER)
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QTimer>
#include <QUrl>
#endif

#if defined(USE_KDSINGLEAPPLICATION) &&                                        \
  (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QBuffer>
#include <kdsingleapplication.h>
#endif

#ifdef Q_OS_WIN
#include "core/globalshortcutfilter.h"
#endif

namespace {
/**
 * @brief Read a pin message written by FlameshotDaemon::createPin.
 *
 * A QPixmap loses its device pixel ratio when it goes through a QDataStream,
 * so it travels next to the pixmap and is restored here. Without it the pin is
 * laid out in device pixels and comes out too big on a scaled screen. Messages
 * from an older flameshot don't carry it, in which case the pixmap keeps the
 * ratio it was created with.
 */
void readPin(QDataStream& stream, QPixmap& pixmap, QRect& geometry)
{
    qreal devicePixelRatio = 0;
    stream >> pixmap >> geometry >> devicePixelRatio;
    if (stream.status() == QDataStream::Ok && devicePixelRatio > 0) {
        pixmap.setDevicePixelRatio(devicePixelRatio);
    }
}
}

/**
 * @brief A way of accessing the flameshot daemon both from the daemon itself,
 * and from subcommands.
 *
 * The daemon is necessary in order to:
 * - Host the system tray,
 * - Listen for hotkey events that will trigger captures,
 * - Host pinned screenshot widgets,
 * - Host the clipboard on X11, where the clipboard gets lost once flameshot
 *   quits.
 *
 * If the `autoCloseIdleDaemon` option is true, the daemon will close as soon as
 * it is not needed to host pinned screenshots and the clipboard.
 *
 * Both the daemon and non-daemon flameshot processes use the same public API,
 * which is implemented as static methods. In the daemon process, this class is
 * also instantiated as a singleton, so it can listen to D-Bus calls via the
 * sigslot mechanism. The instantiation is done by calling `start` (this must be
 * done only in the daemon process). Any instance (as opposed to static) members
 * can only be used if the current process is a daemon.
 *
 * @note The daemon will be automatically launched where necessary, via D-Bus.
 * This applies only to Linux.
 */
FlameshotDaemon::FlameshotDaemon()
  : m_persist(false)
  , m_hostingClipboard(false)
  , m_clipboardSignalBlocked(false)
  , m_trayIcon(nullptr)
#if !defined(DISABLE_UPDATE_CHECKER)
  , m_appLatestVersion(QStringLiteral(APP_VERSION).replace("v", ""))
  , m_showManualCheckAppUpdateStatus(false)
  , m_updateCheckInProgress(false)
  , m_updateCheckTimer(nullptr)
  , m_updateCheckThread(nullptr)
#endif
#ifdef Q_OS_WIN
  , m_nativeFilter(nullptr)
#endif
{
    connect(
      QApplication::clipboard(), &QClipboard::dataChanged, this, [this]() {
          if (!m_hostingClipboard || m_clipboardSignalBlocked) {
              m_clipboardSignalBlocked = false;
              return;
          }
          m_hostingClipboard = false;
          quitIfIdle();
      });

    m_persist = !ConfigHandler().autoCloseIdleDaemon();
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::fileChanged,
            this,
            [this]() {
                ConfigHandler config;
                enableTrayIcon(!config.disabledTrayIcon());
                m_persist = !config.autoCloseIdleDaemon();
            });

#if !defined(DISABLE_UPDATE_CHECKER)
    m_updateCheckTimer = new QTimer(this);
    m_updateCheckTimer->setInterval(1000 * 60 * 60 * 24);
    connect(m_updateCheckTimer, &QTimer::timeout, this, [this]() {
        if (ConfigHandler().checkForUpdates()) {
            getLatestAvailableVersion();
        }
    });
    m_updateCheckTimer->start();

    if (ConfigHandler().checkForUpdates()) {
        // Defer startup update check so proxy/network/TLS init cannot stall
        // the GUI thread during startup, first hotkey, or tray menu opening.
        QTimer::singleShot(3000, this, [this]() {
            if (ConfigHandler().checkForUpdates()) {
                getLatestAvailableVersion();
            }
        });
    }
#endif
}

FlameshotDaemon::~FlameshotDaemon()
{
#if !defined(DISABLE_UPDATE_CHECKER)
    if (m_updateCheckThread) {
        QThread* thread = m_updateCheckThread;
        m_updateCheckThread = nullptr;
        disconnect(thread, nullptr, this, nullptr);
        thread->quit();
        thread->wait(1500);
    }
#endif
#ifdef Q_OS_WIN
    if (qApp && m_nativeFilter) {
        qApp->removeNativeEventFilter(m_nativeFilter);
        m_nativeFilter = nullptr;
    }
#endif
    if (m_trayIcon) {
        m_trayIcon->hide();
        delete m_trayIcon;
        m_trayIcon = nullptr;
    }
    if (m_instance == this) {
        m_instance = nullptr;
    }
}

void FlameshotDaemon::start()
{
    if (!m_instance) {
        m_instance = new FlameshotDaemon();
        // Tray icon needs FlameshotDaemon::instance() to be non-null
        m_instance->initTrayIcon();
        qApp->setQuitOnLastWindowClosed(false);
        connect(qApp, &QCoreApplication::aboutToQuit, m_instance, []() {
            if (m_instance) {
                delete m_instance;
                m_instance = nullptr;
            }
        });
    }
}

void FlameshotDaemon::createPin(const QPixmap& capture, QRect geometry)
{
    if (instance()) {
        instance()->attachPin(capture, geometry);
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // A QPixmap loses its device pixel ratio when streamed, so send it along.
#if defined(USE_KDSINGLEAPPLICATION) &&                                        \
  (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    auto kdsa = KDSingleApplication(QStringLiteral("org.flameshot.Flameshot"));
    stream << QStringLiteral("attachPin") << capture << geometry
           << capture.devicePixelRatio();
    kdsa.sendMessage(data);
#else
    stream << capture << geometry << capture.devicePixelRatio();
    QDBusMessage m = createMethodCall(QStringLiteral("attachPin"));
    m << data;
    call(m);
#endif
}

void FlameshotDaemon::copyToClipboard(const QPixmap& capture)
{
    if (instance()) {
        instance()->attachScreenshotToClipboard(capture);
        return;
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

#if defined(USE_KDSINGLEAPPLICATION) &&                                        \
  (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    auto kdsa = KDSingleApplication(QStringLiteral("org.flameshot.Flameshot"));
    stream << QStringLiteral("attachScreenshotToClipboard") << capture;
    kdsa.sendMessage(data);
#else
    stream << capture;
    QDBusMessage m =
      createMethodCall(QStringLiteral("attachScreenshotToClipboard"));

    m << data;
    call(m);
#endif
}

void FlameshotDaemon::copyToClipboard(const QString& text,
                                      const QString& notification)
{
    if (instance()) {
        instance()->attachTextToClipboard(text, notification);
        return;
    }

#if defined(USE_KDSINGLEAPPLICATION) &&                                        \
  (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    auto kdsa = KDSingleApplication(QStringLiteral("org.flameshot.Flameshot"));
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QStringLiteral("attachTextToClipboard") << text << notification;
    kdsa.sendMessage(data);
#else
    auto m = createMethodCall(QStringLiteral("attachTextToClipboard"));
    m << text << notification;
    call(m);
#endif
}

/**
 * @brief Is this instance of flameshot hosting any windows as a daemon?
 */
bool FlameshotDaemon::isThisInstanceHostingWidgets()
{
    return instance() && !instance()->m_widgets.isEmpty();
}

void FlameshotDaemon::sendTrayNotification(const QString& text,
                                           const QString& title,
                                           const int timeout)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(
          title, text, QIcon(GlobalValues::iconPath()), timeout);
    }
}

#if !defined(DISABLE_UPDATE_CHECKER)
void FlameshotDaemon::showUpdateNotificationIfAvailable(CaptureWidget* widget)
{
    if (!m_appLatestUrl.isEmpty() &&
        ConfigHandler().ignoreUpdateToVersion().compare(m_appLatestVersion) <
          0) {
        widget->showAppUpdateNotification(m_appLatestVersion, m_appLatestUrl);
    }
}

void FlameshotDaemon::getLatestAvailableVersion()
{
    if (m_updateCheckInProgress) {
        return;
    }
    m_updateCheckInProgress = true;

    struct UpdateCheckResult
    {
        bool completed = false;
        bool ok = false;
        QString errorString;
        QByteArray payload;
    };

    // Run update check (and synchronous Windows WPAD/proxy resolution) on a
    // dedicated worker thread so the main GUI thread is never stalled.
    auto result = std::make_shared<UpdateCheckResult>();
    auto* thread = new QThread();
    auto* worker = new QObject();
    worker->moveToThread(thread);
    m_updateCheckThread = thread;

    connect(thread, &QThread::started, worker, [worker, result]() {
        auto* nam = new QNetworkAccessManager(worker);
        QNetworkRequest requestCheckUpdates(QUrl(FLAMESHOT_APP_VERSION_URL));
        QNetworkReply* reply = nam->get(requestCheckUpdates);
        connect(
          reply, &QNetworkReply::finished, worker, [worker, reply, result]() {
              result->ok = (reply->error() == QNetworkReply::NoError);
              result->errorString = reply->errorString();
              result->payload = result->ok ? reply->readAll() : QByteArray();
              result->completed = true;
              reply->deleteLater();
              worker->deleteLater();
          });
    });

    connect(worker,
            &QObject::destroyed,
            thread,
            &QThread::quit,
            Qt::DirectConnection);
    connect(thread, &QThread::finished, this, [this, thread, result]() {
        if (m_updateCheckThread == thread) {
            m_updateCheckInProgress = false;
            m_updateCheckThread = nullptr;
        }
        if (result->completed) {
            handleReplyCheckUpdates(
              result->ok, result->errorString, result->payload);
        }
    });
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    if (qApp) {
        connect(qApp, &QCoreApplication::aboutToQuit, thread, [thread]() {
            thread->quit();
        });
    }
    thread->start();
}

void FlameshotDaemon::checkForUpdates()
{
    bool autoCheckEnabled = ConfigHandler().checkForUpdates();

    if (autoCheckEnabled) {
        if (!m_appLatestUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(m_appLatestUrl));
        }
    } else {
        m_showManualCheckAppUpdateStatus = true;

        if (m_appLatestUrl.isEmpty()) {
            getLatestAvailableVersion();
        } else {
            QVersionNumber appLatestVersion =
              QVersionNumber::fromString(m_appLatestVersion);
            if (Flameshot::instance()->getVersion() < appLatestVersion) {
                QDesktopServices::openUrl(QUrl(m_appLatestUrl));
            } else {
                sendTrayNotification(tr("You have the latest version"),
                                     "Flameshot");
            }
        }
    }
}
#endif

/**
 * @brief Return the daemon instance.
 *
 * If this instance of flameshot is the daemon, a singleton instance of
 * `FlameshotDaemon` is returned. As a side effect`start` will called if it
 * wasn't called earlier. If this instance of flameshot is not the daemon,
 * `nullptr` is returned.
 *
 * This strategy is used because the daemon needs to receive signals from D-Bus,
 * for which an instance of a `QObject` is required. The singleton serves as
 * that object.
 */
FlameshotDaemon* FlameshotDaemon::instance()
{
    // Because we don't use DBus on MacOS, each instance of flameshot is its own
    // mini-daemon, responsible for hosting its own persistent widgets (e.g.
    // pins).
#if defined(Q_OS_MACOS)
    start();
#endif
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
        qApp->exit(E_OK);
    }
}

// SERVICE METHODS

void FlameshotDaemon::attachPin(const QPixmap& pixmap, QRect geometry)
{
    auto* pinWidget = new PinWidget(pixmap, geometry);
    m_widgets.append(pinWidget);
    connect(pinWidget, &QObject::destroyed, this, [=, this]() {
        m_widgets.removeOne(pinWidget);
        quitIfIdle();
    });

    pinWidget->show();
    pinWidget->activateWindow();
}

void FlameshotDaemon::attachScreenshotToClipboard(const QPixmap& pixmap)
{
    m_hostingClipboard = true;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->blockSignals(true);
    // This variable is necessary because the signal doesn't get blocked on
    // windows for some reason
    m_clipboardSignalBlocked = true;
    saveToClipboard(pixmap);
    clipboard->blockSignals(false);
}

// D-BUS / KDSingleApplication METHODS

void FlameshotDaemon::attachPin(const QByteArray& data)
{
    QDataStream stream(data);
    QPixmap pixmap;
    QRect geometry;

    readPin(stream, pixmap, geometry);

    attachPin(pixmap, geometry);
}

void FlameshotDaemon::attachScreenshotToClipboard(const QByteArray& screenshot)
{
    QDataStream stream(screenshot);
    QPixmap p;
    stream >> p;

    attachScreenshotToClipboard(p);
}

void FlameshotDaemon::attachTextToClipboard(const QString& text,
                                            const QString& notification)
{
    // Must send notification before clipboard modification on linux
    if (!notification.isEmpty()) {
        AbstractLogger::info() << notification;
    }

    m_hostingClipboard = true;
    QClipboard* clipboard = QApplication::clipboard();

    clipboard->blockSignals(true);
    // This variable is necessary because the signal doesn't get blocked on
    // windows for some reason
    m_clipboardSignalBlocked = true;
    clipboard->setText(text);
    clipboard->blockSignals(false);
}

void FlameshotDaemon::initTrayIcon()
{
    if (!ConfigHandler().disabledTrayIcon()) {
        enableTrayIcon(true);
    }
#if defined(Q_OS_WIN)
    m_nativeFilter = new GlobalShortcutFilter(this);
    qApp->installNativeEventFilter(m_nativeFilter);
#endif
}

void FlameshotDaemon::enableTrayIcon(bool enable)
{
    if (enable) {
        if (m_trayIcon == nullptr) {
            m_trayIcon = new TrayIcon(this);
        } else if (!m_trayIcon->isVisible()) {
            m_trayIcon->show();
        }
    } else if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->hide();
    }
}

#if !defined(DISABLE_UPDATE_CHECKER)
void FlameshotDaemon::handleReplyCheckUpdates(bool ok,
                                              const QString& errorString,
                                              const QByteArray& payload)
{
    if (!ConfigHandler().checkForUpdates() &&
        !m_showManualCheckAppUpdateStatus) {
        return;
    }

    if (ok) {
        QJsonDocument response = QJsonDocument::fromJson(payload);
        QJsonObject json = response.object();
        m_appLatestVersion = json["tag_name"].toString().replace("v", "");

        QVersionNumber appLatestVersion =
          QVersionNumber::fromString(m_appLatestVersion);
        if (Flameshot::instance()->getVersion() < appLatestVersion) {
            emit newVersionAvailable(appLatestVersion);
            m_appLatestUrl = json["html_url"].toString();
            if (m_showManualCheckAppUpdateStatus) {
                QDesktopServices::openUrl(QUrl(m_appLatestUrl));
            }
        } else if (m_showManualCheckAppUpdateStatus) {
            sendTrayNotification(tr("You have the latest version"),
                                 "Flameshot");
        }
    } else {
        qWarning() << "Failed to get information about the latest version. "
                   << errorString;
        if (m_showManualCheckAppUpdateStatus) {
            if (FlameshotDaemon::instance()) {
                FlameshotDaemon::instance()->sendTrayNotification(
                  tr("Failed to get information about the latest version."),
                  "Flameshot");
            }
        }
    }
    m_showManualCheckAppUpdateStatus = false;
}
#endif

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
QDBusMessage FlameshotDaemon::createMethodCall(const QString& method)
{
    QDBusMessage m =
      QDBusMessage::createMethodCall(QStringLiteral("org.flameshot.Flameshot"),
                                     QStringLiteral("/"),
                                     QLatin1String(""),
                                     method);
    return m;
}

void FlameshotDaemon::checkDBusConnection(const QDBusConnection& connection)
{
    if (!connection.isConnected()) {
        AbstractLogger::error() << tr("Unable to connect via DBus");
        qApp->exit(E_DBUSCONN);
    }
}

void FlameshotDaemon::call(const QDBusMessage& m)
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    checkDBusConnection(sessionBus);
    sessionBus.call(m);
}
#endif

#if defined(USE_KDSINGLEAPPLICATION) &&                                        \
  (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
void FlameshotDaemon::messageReceivedFromSecondaryInstance(
  const QByteArray& message)
{
    // qDebug() << "Received message from second instance:" << message;

    QByteArray messageCopy = message;
    QBuffer buffer(&messageCopy);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    QString methodCall;
    stream >> methodCall;
    // qDebug() << "Method:" << methodCall;

    if (methodCall == QStringLiteral("attachPin")) {
        QPixmap capture;
        QRect geometry;
        readPin(stream, capture, geometry);
        // qDebug() << "Pixmap:" << capture;
        // qDebug() << "Geometry:" << geometry;
        if (!capture.isNull()) {
            FlameshotDaemon::instance()->attachPin(capture, geometry);
        } else {
            qWarning() << "Received \"attachPin\" from second instance, but "
                          "pixmap is empty!";
        }
    } else if (methodCall == QStringLiteral("attachScreenshotToClipboard")) {
        QPixmap capture;
        stream >> capture;
        // qDebug() << "Pixmap:" << capture;
        if (!capture.isNull()) {
            FlameshotDaemon::instance()->attachScreenshotToClipboard(capture);
        } else {
            qWarning() << "Received \"attachScreenshotToClipboard\" from "
                          "second instance, but pixmap is empty!";
        }
    } else if (methodCall == (QStringLiteral("attachTextToClipboard"))) {
        QString text;
        QString notification;
        stream >> text >> notification;
        // qDebug() << "Text:" << text;
        // qDebug() << "Notification:" << notification;
        if (!text.isEmpty()) {
            FlameshotDaemon::instance()->attachTextToClipboard(text,
                                                               notification);
        } else {
            qWarning() << "Received \"attachTextToClipboard\" from second "
                          "instance, but text is empty!";
        }
    } else {
        qWarning() << "Received unknown message from second instance:"
                   << message;
    }
}
#endif

// STATIC ATTRIBUTES
FlameshotDaemon* FlameshotDaemon::m_instance = nullptr;
