#pragma once

#include <QByteArray>
#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class QPixmap;
class QRect;

class FlameshotDaemon : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.flameshot.Flameshot")
public:
    static void start();
    static FlameshotDaemon* instance();
    static void createPin(QPixmap capture, QRect geometry);
    static void copyToClipboard(QPixmap capture);
    // TODO static void createAppLauncher();
private:
    FlameshotDaemon();
    void quitIfIdle();
    void attachPin(QPixmap pixmap, QRect geometry);
    void attachClipboard(QPixmap pixmap);

private slots:
    Q_NOREPLY void attachPin(const QByteArray& data);
    Q_NOREPLY void attachClipboard(const QByteArray& screenshot);
    void onClipboardChanged();

private:
    // TODO toggle this using a config option?
    bool m_persist;
    bool m_hostingClipboard;
    QList<QWidget*> m_widgets;
    static FlameshotDaemon* m_instance;

    // TODO temporary
    friend class FlameshotDBusAdapter;
};
