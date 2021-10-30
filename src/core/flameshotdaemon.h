#pragma once

#include <QByteArray>
#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class QPixmap;
class QRect;
class QDBusMessage;
class QDBusConnection;

class FlameshotDaemon : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.flameshot.Flameshot")
public:
    static void start();
    static FlameshotDaemon* instance();
    static void createPin(QPixmap capture, QRect geometry);
    static void copyToClipboard(QPixmap capture);
    static void copyToClipboard(QString text);
    static void enableTrayIcon(bool enable);
    // TODO static void createAppLauncher();
private:
    FlameshotDaemon();
    void quitIfIdle();
    void attachPin(QPixmap pixmap, QRect geometry);
    void attachScreenshotToClipboard(QPixmap pixmap);

private slots:
    Q_NOREPLY void attachPin(const QByteArray& data);
    Q_NOREPLY void attachScreenshotToClipboard(const QByteArray& screenshot);
    Q_NOREPLY void attachTextToClipboard(QString text);

private:
    static QDBusMessage createMethodCall(QString method);
    static void checkDBusConnection(const QDBusConnection& connection);
    static void call(const QDBusMessage& m);

private:
    // TODO toggle this using a config option?
    bool m_persist;
    bool m_hostingClipboard;
    bool m_clipboardSignalBlocked;
    QList<QWidget*> m_widgets;
    static FlameshotDaemon* m_instance;

    // TODO temporary
    friend class FlameshotDBusAdapter;
};
