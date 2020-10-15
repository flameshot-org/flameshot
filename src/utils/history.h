#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX_SIZE 25

#include <QList>
#include <QPixmap>
#include <QString>

struct HISTORY_FILE_NAME
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

    const HISTORY_FILE_NAME& unpackFileName(const QString&);
    const QString& packFileName(const QString&, const QString&, const QString&);

private:
    QString m_historyPath;
    QList<QString> m_thumbs;

    // temporary variables
    QString m_packedFileName;
    HISTORY_FILE_NAME m_unpackedFileName;
};

#endif // HISTORY_H
