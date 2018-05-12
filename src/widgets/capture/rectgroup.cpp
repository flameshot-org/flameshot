#include "rectgroup.h"

RectGroup::RectGroup() {

}

RectGroup::RectGroup(const QVector<QRect> &v) : m_rects(v) {

}

void RectGroup::setRects(const QVector<QRect> &v) {
    m_rects = v;
}

QRect RectGroup::getRectContainingPoint(const QPoint &p) const {
    QRect res;
    for (const QRect &r : m_rects) {
        if (r.contains(p)) {
            res = r;
            break;
        }
    }
    return res;
}
