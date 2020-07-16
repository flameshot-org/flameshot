#ifndef HISTORY_H
#define HISTORY_H

#include <QList>

#define HISTORY_MAX_SIZE 10
#define HISTORY_THUNB_SCALE 1.5
#define HISTORY_THUNB_WIDTH 160*HISTORY_THUNB_SCALE
#define HISTORY_THUNB_HEIGH 90*HISTORY_THUNB_SCALE


class QPixmap;
class QString;


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
