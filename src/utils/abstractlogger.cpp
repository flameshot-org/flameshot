#include "abstractlogger.h"
#include "systemnotification.h"
#include <cassert>

#include <QFileInfo>

AbstractLogger::AbstractLogger(Channel channel, int targets)
  : m_defaultChannel(channel)
  , m_targets(targets)
{
    if (targets & LogFile) {
        // TODO
    }
}

/**
 * @brief Construct an AbstractLogger with output to a string.
 * @param additionalChannels Optional additional targets to output to.
 */
AbstractLogger::AbstractLogger(QString& str,
                               Channel channel,
                               int additionalChannels)
  : AbstractLogger(channel, additionalChannels)
{
    m_textStreams << new QTextStream(&str);
}

AbstractLogger::~AbstractLogger()
{
    qDeleteAll(m_textStreams);
}

AbstractLogger AbstractLogger::info(int targets)
{
    return { Info, targets };
}

AbstractLogger AbstractLogger::warning(int targets)
{
    return { Warning, targets };
}

AbstractLogger AbstractLogger::error(int targets)
{
    return { Error, targets };
}

AbstractLogger& AbstractLogger::sendMessage(QString msg, Channel channel)
{
    if (m_targets & Notification) {
        SystemNotification().sendMessage(
          msg, messageHeader(channel, Notification), m_notificationPath);
    }
    if (!m_textStreams.isEmpty()) {
        foreach (auto* stream, m_textStreams) {
            *stream << messageHeader(channel, String) << msg << "\n";
        }
    }
    if (m_targets & LogFile) {
        // TODO
    }
    if (m_targets & Stderr) {
        QTextStream stream(stderr);
        stream << messageHeader(channel, Stderr) << msg << "\n";
    }
    return *this;
}

/**
 * @brief Send a message to the default channel of this logger.
 * @param msg
 * @return
 */
AbstractLogger& AbstractLogger::operator<<(QString msg)
{
    sendMessage(msg, m_defaultChannel);
    return *this;
}

AbstractLogger& AbstractLogger::addOutputString(QString& str)
{
    m_textStreams << new QTextStream(&str);
    return *this;
}

/**
 * @brief Attach a path to a notification so it can be dragged and dropped.
 */
AbstractLogger& AbstractLogger::attachNotificationPath(QString path)
{
    if (m_targets & Notification) {
        m_notificationPath = path;
    } else {
        assert("Cannot attach notification path to a logger without a "
               "notification channel.");
    }
    return *this;
}

/**
 * @brief Enable/disable message header (e.g. "flameshot: info:").
 */
AbstractLogger& AbstractLogger::enableMessageHeader(bool enable)
{
    m_enableMessageHeader = enable;
    return *this;
}

/**
 * @brief Generate a message header for the given channel and target.
 */
QString AbstractLogger::messageHeader(Channel channel, Target target)
{
    if (!m_enableMessageHeader) {
        return "";
    }
    QString messageChannel;
    if (channel == Info) {
        messageChannel = "info";
    } else if (channel == Warning) {
        messageChannel = "warning";
    } else if (channel == Error) {
        messageChannel = "error";
    }

    if (target == Notification) {
        messageChannel[0] = messageChannel[0].toUpper();
        return "Flameshot " + messageChannel;
    } else {
        return "flameshot: " + messageChannel + ": ";
    }
}
