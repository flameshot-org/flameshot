#include "logfile.h"

#include "confighandler.h"

#include <QDir>
#include <QMutexLocker>
#include <QStandardPaths>

constexpr const char* LOG_FILE_NAME = "flameshot.log";
constexpr const char* OLD_LOG_FILE_NAME = "flameshot.log.old";

void LogFile::write(const QString* data)
{
    QMutexLocker locker(&m_mutex);
    if (nullptr == m_instance) {
        m_instance = new LogFile();
    }

    m_instance->writeToFile(data);
}

void LogFile::resetLogFile()
{
    QMutexLocker locker(&m_mutex);
    delete m_instance;
    m_instance = nullptr;
}

QString LogFile::defaultLogFilePath()
{
#ifdef QT_STATE_DIR_SUPPORTED
    const auto defaultLocation = QStandardPaths::GenericStateLocation;
#else
    const auto defaultLocation = QStandardPaths::GenericDataLocation;
#endif
    return QDir::toNativeSeparators(
      QStandardPaths::writableLocation(defaultLocation) + "/flameshot");
}

LogFile::LogFile()
{
    auto loggingDirectory = QDir(ConfigHandler().logFilePath());

    if (!loggingDirectory.exists()) {
        loggingDirectory.mkpath(".");
    }

    if (loggingDirectory.exists(OLD_LOG_FILE_NAME)) {
        loggingDirectory.remove(OLD_LOG_FILE_NAME);
    }

    if (loggingDirectory.exists(LOG_FILE_NAME)) {
        loggingDirectory.rename(LOG_FILE_NAME, OLD_LOG_FILE_NAME);
    }

    auto logFilePath = loggingDirectory.filePath(LOG_FILE_NAME);
    m_logFile = std::make_unique<QFile>(logFilePath);
    if (m_logFile->open(QIODeviceBase::WriteOnly | QIODeviceBase::Append)) {
        m_writer = std::make_unique<QTextStream>();
        m_writer->setDevice(m_logFile.get());
    }
}

void LogFile::writeToFile(const QString* data)
{
    if (m_writer) {
        *m_writer << *data;
        m_writer->flush();
    }
}

LogFile* LogFile::m_instance = nullptr;
QMutex LogFile::m_mutex;
