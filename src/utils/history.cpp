#include "history.h"
#include "src/utils/confighandler.h"
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QStringList>

History::History()
{
    // Get cache history path
    ConfigHandler config;
#ifdef Q_OS_WIN
    m_historyPath = QDir::homePath() + "/AppData/Roaming/flameshot/history/";
#else
    QString path = QProcessEnvironment::systemEnvironment().value(
      "XDG_CACHE_HOME", QDir::homePath() + "/.cache");
    m_historyPath = path + "/flameshot/history/";
#endif

    // Check if directory for history exists and create if doesn't
    QDir dir = QDir(m_historyPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

const QString& History::path()
{
    return m_historyPath;
}

void History::save(const QPixmap& pixmap, const QString& fileName)
{
    // scale preview only in local disk
    QPixmap pixmapScaled = QPixmap(pixmap);
    if (pixmap.height() / HISTORYPIXMAP_MAX_PREVIEW_HEIGHT >=
        pixmap.width() / HISTORYPIXMAP_MAX_PREVIEW_WIDTH) {
        pixmapScaled = pixmap.scaledToHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT,
                                             Qt::SmoothTransformation);
    } else {
        pixmapScaled = pixmap.scaledToWidth(HISTORYPIXMAP_MAX_PREVIEW_WIDTH,
                                            Qt::SmoothTransformation);
    }

    // save preview
    QFile file(path() + fileName);
    file.open(QIODevice::WriteOnly);
    pixmapScaled.save(&file, "PNG");

    history();
}

const QList<QString>& History::history()
{
    QDir directory(path());
    QStringList images = directory.entryList(QStringList() << "*.png"
                                                           << "*.PNG",
                                             QDir::Files,
                                             QDir::Time);
    int cnt = 0;
    int max = ConfigHandler().uploadHistoryMax();
    m_thumbs.clear();
    foreach (QString fileName, images) {
        if (++cnt <= max) {
            m_thumbs.append(fileName);
        } else {
            QFile file(path() + fileName);
            file.remove();
        }
    }
    return m_thumbs;
}

const HistoryFileName& History::unpackFileName(const QString& fileNamePacked)
{
    int nPathIndex = fileNamePacked.lastIndexOf("/");
    QStringList unpackedFileName;
    if (nPathIndex == -1) {
        unpackedFileName = fileNamePacked.split("-");
    } else {
        unpackedFileName = fileNamePacked.mid(nPathIndex + 1).split("-");
    }

    switch (unpackedFileName.length()) {
        case 3:
            m_unpackedFileName.file = unpackedFileName[2];
            m_unpackedFileName.token = unpackedFileName[1];
            m_unpackedFileName.type = unpackedFileName[0];
            break;
        case 2:
            m_unpackedFileName.file = unpackedFileName[1];
            m_unpackedFileName.token = "";
            m_unpackedFileName.type = unpackedFileName[0];
            break;
        default:
            m_unpackedFileName.file = unpackedFileName[0];
            m_unpackedFileName.token = "";
            m_unpackedFileName.type = "";
            break;
    }
    return m_unpackedFileName;
}

const QString& History::packFileName(const QString& storageType,
                                     const QString& deleteToken,
                                     const QString& fileName)
{
    m_packedFileName = fileName;
    if (storageType.length() > 0) {
        if (deleteToken.length() > 0) {
            m_packedFileName =
              storageType + "-" + deleteToken + "-" + m_packedFileName;
        } else {
            m_packedFileName = storageType + "-" + m_packedFileName;
        }
    }
    return m_packedFileName;
}
