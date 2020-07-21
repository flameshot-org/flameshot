#include "history.h"
#include "src/utils/confighandler.h"
#include <QDir>
#include <QFile>
#include <QDebug>


History::History()
{
    // Get cache history path
    ConfigHandler config;
    QString userUuid = config.userUuid();
    m_historyPath = QDir::homePath() + "/.cache/flameshot/history/"+ userUuid + "/";

    // Check if directory for history exists and create if doesn't
    QDir dir = QDir(m_historyPath);
    if (!dir.exists())
        dir.mkpath(".");
}

const QString &History::path() {
    return m_historyPath;
}

void History::save(const QPixmap &pixmap, const QString &fileName) {
    QFile file(path() + fileName);
    file.open(QIODevice::WriteOnly);
    pixmap.scaled(HISTORY_THUNB_WIDTH,
                  HISTORY_THUNB_HEIGH,
                  Qt::KeepAspectRatio,
                  Qt::SmoothTransformation
                  ).save(&file, "PNG");
    history();
}

const QList<QString> &History::history() {
    QDir directory(path());
    QStringList images = directory.entryList(QStringList() << "*.png" << "*.PNG", QDir::Files, QDir::Time);
    int cnt = 0;
    m_thumbs.clear();
    foreach(QString fileName, images) {
        if(++cnt <= HISTORY_MAX_SIZE) {
            m_thumbs.append(fileName);
        }
        else {
            QFile file(path() + fileName);
            file.remove();
        }
    }
    return m_thumbs;
}
