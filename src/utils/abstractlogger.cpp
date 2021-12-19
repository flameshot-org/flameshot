#include "abstractlogger.h"
#include "systemnotification.h"

#include <QFileInfo>

// TODO add null checks and exceptions
// TODO only send notification when flushed

AbstractLogger::AbstractLogger(Type type, int channels)
  : m_channels(channels)
{
    if (channels & LogFile) {
        // TODO
    }
}

AbstractLogger::AbstractLogger(QString& str, Type type, int additionalChannels)
  : AbstractLogger(type, additionalChannels)
{
    // TODO error if channels excludes String.
    m_textStreams << new QTextStream(&str);
}

AbstractLogger::~AbstractLogger()
{
    qDeleteAll(m_textStreams);
}

AbstractLogger& AbstractLogger::operator<<(const QString& msg)
{
    if (m_channels & Notification) {
        SystemNotification().sendMessage(msg);
    }
    if (!m_textStreams.isEmpty()) {
        foreach (auto* stream, m_textStreams) {
            *stream << msg;
        }
    }
    if (m_channels & LogFile) {
        // TODO
    }
    if (m_channels & Stdout) {
        // TODO this doesn't work with dbus. Either remove dbus or change this.
        // TODO should we ever log to stdout?
        QTextStream stream(stdout);
        stream << msg << "\n";
    }
    if (m_channels & Stderr) {
        // TODO this doesn't work with dbus. Either remove dbus or change this.
        QTextStream stream(stderr);
        stream << msg << "\n";
    }
    return *this;
}

void AbstractLogger::addOutputString(QString& str)
{
    m_textStreams << new QTextStream(&str);
}
