#pragma once

#include <QByteArray>
#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class QPixmap;
class QRect;
class QDBusMessage;
class QDBusConnection;

class FlameshotDaemon : private QObject
{
    Q_OBJECT
public:
    static void start();
    static FlameshotDaemon* instance();
    static void createPin(QPixmap capture, QRect geometry);
    static void copyToClipboard(QPixmap capture);
    static void copyToClipboard(QString text, QString notification = "");
    static void enableTrayIcon(bool enable);
    static bool isThisInstanceHostingWidgets();

private:
    FlameshotDaemon();
    void quitIfIdle();
    void attachPin(QPixmap pixmap, QRect geometry);
    void attachScreenshotToClipboard(QPixmap pixmap);

    void attachPin(const QByteArray& data);
    void attachScreenshotToClipboard(const QByteArray& screenshot);
    void attachTextToClipboard(QString text, QString notification);

private:
    static QDBusMessage createMethodCall(QString method);
    static void checkDBusConnection(const QDBusConnection& connection);
    static void call(const QDBusMessage& m);

    bool m_persist;
    bool m_hostingClipboard;
    bool m_clipboardSignalBlocked;
    QList<QWidget*> m_widgets;
    static FlameshotDaemon* m_instance;

    friend class FlameshotDBusAdapter;
};
