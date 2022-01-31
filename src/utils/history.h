#ifndef HISTORY_H
#define HISTORY_H

#define HISTORYPIXMAP_MAX_PREVIEW_WIDTH 250
#define HISTORYPIXMAP_MAX_PREVIEW_HEIGHT 100

#include <QList>
#include <QPixmap>
#include <QString>

struct HistoryFileName
{
    QString file;
    QString token;
    QString type;
};

class History
{
public:
    History();

    void save(const QPixmap&, const QString&);
    const QList<QString>& history();
    const QString& path();

    const HistoryFileName& unpackFileName(const QString&);
    const QString& packFileName(const QString&, const QString&, const QString&);

private:
    QString m_historyPath;
    QList<QString> m_thumbs;

    // temporary variables
    QString m_packedFileName;
    HistoryFileName m_unpackedFileName;
};

#endif // HISTORY_H
