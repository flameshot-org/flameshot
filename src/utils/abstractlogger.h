#pragma once

#include <QString>
#include <QTextStream>

/**
 * @brief A class that allows you to log events to where they need to go.
 */
class AbstractLogger : public QObject
{
    Q_OBJECT
public:
    enum Channel
    {
        Notification = 0x01,
        LogFile = 0x02,
        Stderr = 0x04,
        Stdout = 0x08,
        String = 0x10,
        Default = Notification | LogFile | Stderr,
    };

    enum Type
    {
        Info,
        Warning,
        Error
    };

    AbstractLogger(Type type = Info, int channels = Stdout);
    AbstractLogger(QString& str, Type type, int additionalChannels = String);
    ~AbstractLogger();

    AbstractLogger& operator<<(const QString& msg);
    void addOutputString(QString& str);

private:
    int m_channels;
    QList<QTextStream*> m_textStreams;
};
