#pragma once

#include <QByteArray>
#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class QPixmap;
class QRect;
class QDBusMessage;
class QDBusConnection;
class TrayIcon;
class CaptureWidget;

#if !defined(DISABLE_UPDATE_CHECKER)
class QNetworkAccessManager;
class QNetworkReply;
class QVersionNumber;
#endif

class FlameshotDaemon : public QObject
{
    Q_OBJECT
public:
    static void start();
    static FlameshotDaemon* instance();
    static void createPin(QPixmap capture, QRect geometry);
    static void copyToClipboard(QPixmap capture);
    static void copyToClipboard(QString text, QString notification = "");
    static bool isThisInstanceHostingWidgets();

    void sendTrayNotification(
      const QString& text,
      const QString& title = QStringLiteral("Flameshot Info"),
      const int timeout = 5000);

#if !defined(DISABLE_UPDATE_CHECKER)
    void showUpdateNotificationIfAvailable(CaptureWidget* widget);

public slots:
    void checkForUpdates();
    void getLatestAvailableVersion();

private slots:
    void handleReplyCheckUpdates(QNetworkReply* reply);

signals:
    void newVersionAvailable(QVersionNumber version);
#endif

private:
    FlameshotDaemon();
    void quitIfIdle();
    void attachPin(QPixmap pixmap, QRect geometry);
    void attachScreenshotToClipboard(QPixmap pixmap);

    void attachPin(const QByteArray& data);
    void attachScreenshotToClipboard(const QByteArray& screenshot);
    void attachTextToClipboard(QString text, QString notification);

    void initTrayIcon();
    void enableTrayIcon(bool enable);

    static QDBusMessage createMethodCall(QString method);
    static void checkDBusConnection(const QDBusConnection& connection);
    static void call(const QDBusMessage& m);

    bool m_persist;
    bool m_hostingClipboard;
    bool m_clipboardSignalBlocked;
    QList<QWidget*> m_widgets;
    TrayIcon* m_trayIcon;

#if !defined(DISABLE_UPDATE_CHECKER)
    QString m_appLatestUrl;
    QString m_appLatestVersion;
    bool m_showCheckAppUpdateStatus;
    QNetworkAccessManager* m_networkCheckUpdates;
#endif

    static FlameshotDaemon* m_instance;

    friend class FlameshotDBusAdapter;
};
