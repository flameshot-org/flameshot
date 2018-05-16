#include "rectgroup.h"
#include <algorithm>

RectGroup::RectGroup() {

}

RectGroup::RectGroup(const QVector<QRect> &v) : m_rects(v) {

}

void RectGroup::setRects(const QVector<QRect> &v) {
    m_rects = v;
    std::sort(m_rects.begin(), m_rects.end(),
              [](const QRect &r1, const QRect &r2) {
        return r1.x() < r2.x();
    });
}

QRect RectGroup::getRectContainingPoint(const QPoint &p) const {
    QRect res;
    auto it = std::upper_bound(m_rects.begin(), m_rects.end(), QRect(p, p),
                               [](const QRect &r1, const QRect &r2) {
        return r1.x() < r2.x();
    });

    for (auto i = m_rects.begin(); i != it; i++) {
        if (i->contains(p)) {
            res = (*i);
            break;
        }
    }
    return res;
}
