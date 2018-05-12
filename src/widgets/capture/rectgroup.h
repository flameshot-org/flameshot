#pragma once

#include <QVector>
#include <QRect>

class RectGroup {
public:
    RectGroup();
    RectGroup(const QVector<QRect> &v);

    void setRects(const QVector<QRect> &v);

    QRect getRectContainingPoint(const QPoint &p) const;
private:
    QVector<QRect> m_rects;
};
