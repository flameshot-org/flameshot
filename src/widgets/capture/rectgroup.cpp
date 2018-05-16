#include "rectgroup.h"
#include <algorithm>

RectGroup::RectGroup() {

}

RectGroup::RectGroup(const QVector<QRect> &v) : m_rects(v) {

}

void RectGroup::setRects(const QVector<QRect> &v) {
    m_rects = v;
    // Sort by x coord from higher to lower.
    std::sort(m_rects.begin(), m_rects.end(),
              [](const QRect &r1, const QRect &r2) {
        return r1.x() > r2.x();
    });
}

QRect RectGroup::getRectContainingPoint(const QPoint &p) const {
    QRect res;
    // Get the iterator to the QRect with x() lower than p's x().
    auto it = std::upper_bound(m_rects.begin(), m_rects.end(), QRect(p, p),
                               [](const QRect &r1, const QRect &r2) {
        return r1.x() > r2.x();
    });

    // Find the rect containing the mouse in the rects with x() lower
    // than p's x()
    for (; it != m_rects.end(); it++) {
        if (it->contains(p)) {
            res = (*it);
            break;
        }
    }
    return res;
}
