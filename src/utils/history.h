#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX_SIZE 25

#include <QList>
#include <QString>
#include <QPixmap>


class History
{
public:
    History();

    void save(const QPixmap &, const QString &);
    const QList<QString> &history();
    const QString &path();

private:
    QString m_historyPath;
    QList<QString> m_thumbs;
};

#endif // HISTORY_H
