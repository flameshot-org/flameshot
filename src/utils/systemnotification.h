#ifndef SYSTEMNOTIFICATION_H
#define SYSTEMNOTIFICATION_H

#include <QObject>

class QDBusInterface;

class SystemNotification : public QObject
{
    Q_OBJECT
public:
    explicit SystemNotification(QObject *parent = nullptr);

    void sendMessage(const QString &text,
                     const QString &title = "Flameshot Info",
                     const int timeout = 5000);

signals:

public slots:

private:
    QDBusInterface *m_interface;
};

#endif // SYSTEMNOTIFICATION_H
