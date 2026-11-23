// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "buttonhandler.h"
#include "utils/globalvalues.h"

#include <QPoint>
#include <QScreen>

// ButtonHandler is a handler for every active button. It makes easier to
// manipulate the buttons as a unit.

ButtonHandler::ButtonHandler(const QVector<CaptureToolButton*>& v,
                             QObject* parent)
  : QObject(parent)
{
    setButtons(v);
    init();
}

ButtonHandler::ButtonHandler(QObject* parent)
  : QObject(parent)
{
    init();
}

void ButtonHandler::hide()
{
    for (CaptureToolButton* b : m_vectorButtons) {
        b->hide();
    }
}

void ButtonHandler::show()
{
    if (m_vectorButtons.isEmpty() || m_vectorButtons.first()->isVisible()) {
        return;
    }
    for (CaptureToolButton* b : m_vectorButtons) {
        b->animatedShow();
    }
}

bool ButtonHandler::isVisible() const
{
    bool ret = true;
    for (const CaptureToolButton* b : m_vectorButtons) {
        if (!b->isVisible()) {
            ret = false;
            break;
        }
    }
    return ret;
}

bool ButtonHandler::buttonsAreInside() const
{
    return m_buttonsAreInside;
}

size_t ButtonHandler::size() const
{
    return m_vectorButtons.size();
}

// updatePosition updates the position of the buttons around the
// selection area. Ignores the sides blocked by the end of the screen.
// When the selection is too small, it works on a virtual selection with
// the original in the center.
void ButtonHandler::updatePosition(const QRect& selection)
{
    resetRegionTrack();
    const int vecLength = m_vectorButtons.size();
    if (vecLength == 0) {
        return;
    }
    if (m_screenRegions.size() > 1) {
        updatePositionMultiScreen(selection);
        return;
    }
    // Copy of the selection area for internal modifications
    m_selection = intersectWithAreas(selection);
    updateBlockedSides();
    ensureSelectionMinimumSize();
    // Indicates the actual button to be moved
    int elemIndicator = 0;

    while (elemIndicator < vecLength) {

        // Add them inside the area when there is no more space
        if (m_allSidesBlocked) {
            m_selection = selection;
            positionButtonsInside(elemIndicator);
            break; // the while
        }
        // Number of buttons per row column
        int buttonsPerRow =
          (m_selection.width() + m_separator) / (m_buttonExtendedSize);
        int buttonsPerCol =
          (m_selection.height() + m_separator) / (m_buttonExtendedSize);
        // Buttons to be placed in the corners
        int extraButtons =
          (vecLength - elemIndicator) - (buttonsPerRow + buttonsPerCol) * 2;
        int elemsAtCorners = extraButtons > 4 ? 4 : extraButtons;
        int maxExtra = 2;
        if (m_oneHorizontalBlocked) {
            maxExtra = 1;
        } else if (m_horizontalyBlocked) {
            maxExtra = 0;
        }
        int elemCornersTop = qBound(0, elemsAtCorners, maxExtra);
        elemsAtCorners -= elemCornersTop;
        int elemCornersBotton = qBound(0, elemsAtCorners, maxExtra);

        // Add buttons at the button of the selection
        if (!m_blockedBotton) {
            int addCounter = buttonsPerRow + elemCornersBotton;
            // Don't add more than we have
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);
            QPoint center = QPoint(m_selection.center().x(),
                                   m_selection.bottom() + m_separator);
            if (addCounter > buttonsPerRow) {
                adjustHorizontalCenter(center);
            }
            // ElemIndicator, elemsAtCorners
            QVector<QPoint> positions =
              horizontalPoints(center, addCounter, true);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // Add buttons to the right side of the selection
        if (!m_blockedRight && elemIndicator < vecLength) {
            int addCounter = buttonsPerCol;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);

            QPoint center = QPoint(m_selection.right() + m_separator,
                                   m_selection.center().y());
            QVector<QPoint> positions =
              verticalPoints(center, addCounter, false);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // Add buttons at the top of the selection
        if (!m_blockedTop && elemIndicator < vecLength) {
            int addCounter = buttonsPerRow + elemCornersTop;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);
            QPoint center = QPoint(m_selection.center().x(),
                                   m_selection.top() - m_buttonExtendedSize);
            if (addCounter == 1 + buttonsPerRow) {
                adjustHorizontalCenter(center);
            }
            QVector<QPoint> positions =
              horizontalPoints(center, addCounter, false);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // Add buttons to the left side of the selection
        if (!m_blockedLeft && elemIndicator < vecLength) {
            int addCounter = buttonsPerCol;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);

            QPoint center = QPoint(m_selection.left() - m_buttonExtendedSize,
                                   m_selection.center().y());
            QVector<QPoint> positions =
              verticalPoints(center, addCounter, true);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // If there are elements for the next cycle, increase the size of the
        // base area
        if (elemIndicator < vecLength && !(m_allSidesBlocked)) {
            expandSelection();
        }
        updateBlockedSides();
    }
}

int ButtonHandler::calculateShift(int elements, bool reverse) const
{
    int shift = 0;
    if (elements % 2 == 0) {
        shift = m_buttonExtendedSize * (elements / 2) - (m_separator / 2);
    } else {
        shift =
          m_buttonExtendedSize * ((elements - 1) / 2) + m_buttonBaseSize / 2;
    }
    if (!reverse) {
        shift -= m_buttonBaseSize;
    }

    return shift;
}
// horizontalPoints is an auxiliary method for the button position computation.
// starts from a known center and keeps adding elements horizontally
// and returns the computed positions.
QVector<QPoint> ButtonHandler::horizontalPoints(const QPoint& center,
                                                const int elements,
                                                const bool leftToRight) const
{
    QVector<QPoint> res;
    // Distance from the center to start adding buttons
    int shift = calculateShift(elements, leftToRight);

    int x = leftToRight ? center.x() - shift : center.x() + shift;
    QPoint i(x, center.y());
    while (elements > res.length()) {
        res.append(i);
        leftToRight ? i.setX(i.x() + m_buttonExtendedSize)
                    : i.setX(i.x() - m_buttonExtendedSize);
    }
    return res;
}

// verticalPoints is an auxiliary method for the button position computation.
// starts from a known center and keeps adding elements vertically
// and returns the computed positions.
QVector<QPoint> ButtonHandler::verticalPoints(const QPoint& center,
                                              const int elements,
                                              const bool upToDown) const
{
    QVector<QPoint> res;
    // Distance from the center to start adding buttons
    int shift = calculateShift(elements, upToDown);

    int y = upToDown ? center.y() - shift : center.y() + shift;
    QPoint i(center.x(), y);
    while (elements > res.length()) {
        res.append(i);
        upToDown ? i.setY(i.y() + m_buttonExtendedSize)
                 : i.setY(i.y() - m_buttonExtendedSize);
    }
    return res;
}

QRect ButtonHandler::intersectWithAreas(const QRect& rect)
{
    QRect res;
    for (const QRect& r : m_screenRegions) {
        QRect temp = rect.intersected(r);
        if (temp.height() * temp.width() > res.height() * res.width()) {
            res = temp;
        }
    }
    return res;
}

void ButtonHandler::init()
{
    m_separator = GlobalValues::buttonBaseSize() / 4;
}

void ButtonHandler::resetRegionTrack()
{
    m_buttonsAreInside = false;
}

void ButtonHandler::updatePositionMultiScreen(const QRect& selection)
{
    const int vecLength = m_vectorButtons.size();
    const int extension = m_separator * 2 + m_buttonBaseSize;

    QRegion screenRegion;
    for (const QRect& r : m_screenRegions) {
        screenRegion += r;
    }
    const QRect totalBounds = screenRegion.boundingRect();

    QRect outer = selection;
    if (outer.width() < m_buttonBaseSize) {
        int newX = outer.x() - (m_buttonBaseSize - outer.width()) / 2;
        if (!totalBounds.isEmpty()) {
            newX = qBound(totalBounds.left(),
                          newX,
                          qMax(totalBounds.left(),
                               totalBounds.right() + 1 - m_buttonBaseSize));
        }
        outer.setX(newX);
        outer.setWidth(m_buttonBaseSize);
    }
    if (outer.height() < m_buttonBaseSize) {
        int newY = outer.y() - (m_buttonBaseSize - outer.height()) / 2;
        if (!totalBounds.isEmpty()) {
            newY = qBound(totalBounds.top(),
                          newY,
                          qMax(totalBounds.top(),
                               totalBounds.bottom() + 1 - m_buttonBaseSize));
        }
        outer.setY(newY);
        outer.setHeight(m_buttonBaseSize);
    }

    auto screenSelectionArea = [&](const QRect& r) -> qint64 {
        const QRect inter = selection.intersected(r);
        return inter.isEmpty()
                 ? 0
                 : static_cast<qint64>(inter.width()) * inter.height();
    };

    QRect preferredScreen;
    qint64 bestPreferredArea = 0;
    for (const QRect& r : m_screenRegions) {
        const qint64 area = screenSelectionArea(r);
        if (area > bestPreferredArea) {
            bestPreferredArea = area;
            preferredScreen = r;
        }
    }

    struct HorizontalPlacement
    {
        bool valid = false;
        int rowY = 0;
        int x1 = 0;
        int x2 = 0;
        bool canExtendLeft = false;
        bool canExtendRight = false;
        int buttonsPerRow = 0;
    };

    struct VerticalPlacement
    {
        bool valid = false;
        int colX = 0;
        int y1 = 0;
        int y2 = 0;
        int buttonsPerCol = 0;
    };

    auto findHorizontal = [&](bool isBottom,
                              bool onlyPreferred) -> HorizontalPlacement {
        HorizontalPlacement best;
        qint64 bestScreenArea = -1;
        int bestSpan = 0;
        for (const QRect& r : m_screenRegions) {
            if (onlyPreferred && r != preferredScreen) {
                continue;
            }
            const int rowY =
              isBottom
                ? qMax(outer.bottom() + m_separator, r.top() + m_separator)
                : qMin(outer.top() - m_buttonExtendedSize,
                       r.bottom() + 1 - m_buttonExtendedSize);
            const bool adjacent =
              isBottom ? (rowY <= outer.bottom() + extension)
                       : (rowY + m_buttonBaseSize >= outer.top() - extension);
            const bool fitsVertically =
              isBottom
                ? (rowY >= r.top() &&
                   rowY + m_buttonBaseSize + m_separator - 1 <= r.bottom())
                : (rowY >= r.top() + m_separator &&
                   rowY + m_buttonBaseSize - 1 <= r.bottom());
            if (!adjacent || !fitsVertically) {
                continue;
            }
            const int x1 = qMax(outer.left(), r.left());
            const int x2 = qMin(outer.right(), r.right());
            const int span = x2 - x1 + 1;
            const qint64 area = screenSelectionArea(r);
            if (span >= m_buttonBaseSize &&
                (area > bestScreenArea ||
                 (area == bestScreenArea && span > bestSpan))) {
                bestScreenArea = area;
                bestSpan = span;
                best.valid = true;
                best.rowY = rowY;
                best.x1 = x1;
                best.x2 = x2;
                best.canExtendLeft =
                  (x1 == outer.left()) && (x1 - extension >= r.left());
                best.canExtendRight =
                  (x2 == outer.right()) && (x2 + extension <= r.right());
                best.buttonsPerRow =
                  (span + m_separator) / m_buttonExtendedSize;
            }
        }
        return best;
    };

    auto findVertical = [&](bool isRight,
                            bool onlyPreferred) -> VerticalPlacement {
        VerticalPlacement best;
        qint64 bestScreenArea = -1;
        int bestSpan = 0;
        for (const QRect& r : m_screenRegions) {
            if (onlyPreferred && r != preferredScreen) {
                continue;
            }
            const int colX =
              isRight
                ? qMax(outer.right() + m_separator, r.left() + m_separator)
                : qMin(outer.left() - m_buttonExtendedSize,
                       r.right() + 1 - m_buttonExtendedSize);
            const bool adjacent =
              isRight ? (colX <= outer.right() + extension)
                      : (colX + m_buttonBaseSize >= outer.left() - extension);
            const bool fitsHorizontally =
              isRight ? (colX >= r.left() &&
                         colX + m_buttonBaseSize + m_separator - 1 <= r.right())
                      : (colX >= r.left() + m_separator &&
                         colX + m_buttonBaseSize - 1 <= r.right());
            if (!adjacent || !fitsHorizontally) {
                continue;
            }
            const int y1 = qMax(outer.top(), r.top());
            const int y2 = qMin(outer.bottom(), r.bottom());
            const int span = y2 - y1 + 1;
            const qint64 area = screenSelectionArea(r);
            if (span >= m_buttonBaseSize &&
                (area > bestScreenArea ||
                 (area == bestScreenArea && span > bestSpan))) {
                bestScreenArea = area;
                bestSpan = span;
                best.valid = true;
                best.colX = colX;
                best.y1 = y1;
                best.y2 = y2;
                best.buttonsPerCol =
                  (span + m_separator) / m_buttonExtendedSize;
            }
        }
        return best;
    };

    int elemIndicator = 0;
    auto runPass = [&](bool onlyPreferred) {
        while (elemIndicator < vecLength) {
            const HorizontalPlacement bottom =
              findHorizontal(true, onlyPreferred);
            const VerticalPlacement right = findVertical(true, onlyPreferred);
            const HorizontalPlacement top =
              findHorizontal(false, onlyPreferred);
            const VerticalPlacement left = findVertical(false, onlyPreferred);

            if (!bottom.valid && !right.valid && !top.valid && !left.valid) {
                break;
            }

            const int startIndicator = elemIndicator;
            const int perimeterCapacity =
              (bottom.valid ? bottom.buttonsPerRow : 0) +
              (top.valid ? top.buttonsPerRow : 0) +
              (right.valid ? right.buttonsPerCol : 0) +
              (left.valid ? left.buttonsPerCol : 0);
            const int extraButtons =
              (vecLength - elemIndicator) - perimeterCapacity;
            int elemsAtCorners = extraButtons > 4 ? 4 : qMax(0, extraButtons);

            const int maxExtraTop =
              top.valid
                ? ((top.canExtendLeft ? 1 : 0) + (top.canExtendRight ? 1 : 0))
                : 0;
            const int elemCornersTop = qBound(0, elemsAtCorners, maxExtraTop);
            elemsAtCorners -= elemCornersTop;

            const int maxExtraBottom = bottom.valid
                                         ? ((bottom.canExtendLeft ? 1 : 0) +
                                            (bottom.canExtendRight ? 1 : 0))
                                         : 0;
            const int elemCornersBottom =
              qBound(0, elemsAtCorners, maxExtraBottom);

            QRect nextOuter = outer;

            if (bottom.valid && elemIndicator < vecLength) {
                int addCounter = bottom.buttonsPerRow + elemCornersBottom;
                addCounter = qBound(0, addCounter, vecLength - elemIndicator);
                if (addCounter > 0) {
                    QPoint center((bottom.x1 + bottom.x2) / 2, bottom.rowY);
                    if (addCounter == 1 + bottom.buttonsPerRow) {
                        if (!bottom.canExtendLeft) {
                            center.setX(center.x() + m_buttonExtendedSize / 2);
                        } else if (!bottom.canExtendRight) {
                            center.setX(center.x() - m_buttonExtendedSize / 2);
                        }
                    }
                    moveButtonsToPoints(
                      horizontalPoints(center, addCounter, true),
                      elemIndicator);
                    nextOuter.setBottom(
                      qMax(nextOuter.bottom(), bottom.rowY + m_buttonBaseSize));
                }
            }

            if (right.valid && elemIndicator < vecLength) {
                int addCounter =
                  qBound(0, right.buttonsPerCol, vecLength - elemIndicator);
                if (addCounter > 0) {
                    QPoint center(right.colX, (right.y1 + right.y2) / 2);
                    moveButtonsToPoints(
                      verticalPoints(center, addCounter, false), elemIndicator);
                    nextOuter.setRight(
                      qMax(nextOuter.right(), right.colX + m_buttonBaseSize));
                }
            }

            if (top.valid && elemIndicator < vecLength) {
                int addCounter = top.buttonsPerRow + elemCornersTop;
                addCounter = qBound(0, addCounter, vecLength - elemIndicator);
                if (addCounter > 0) {
                    QPoint center((top.x1 + top.x2) / 2, top.rowY);
                    if (addCounter == 1 + top.buttonsPerRow) {
                        if (!top.canExtendLeft) {
                            center.setX(center.x() + m_buttonExtendedSize / 2);
                        } else if (!top.canExtendRight) {
                            center.setX(center.x() - m_buttonExtendedSize / 2);
                        }
                    }
                    moveButtonsToPoints(
                      horizontalPoints(center, addCounter, false),
                      elemIndicator);
                    nextOuter.setTop(qMin(nextOuter.top(), top.rowY));
                }
            }

            if (left.valid && elemIndicator < vecLength) {
                int addCounter =
                  qBound(0, left.buttonsPerCol, vecLength - elemIndicator);
                if (addCounter > 0) {
                    QPoint center(left.colX, (left.y1 + left.y2) / 2);
                    moveButtonsToPoints(
                      verticalPoints(center, addCounter, true), elemIndicator);
                    nextOuter.setLeft(qMin(nextOuter.left(), left.colX));
                }
            }

            if (elemIndicator == startIndicator) {
                break;
            }
            outer = nextOuter;
        }
    };

    if (!preferredScreen.isEmpty()) {
        runPass(true);
    }
    if (elemIndicator < vecLength) {
        runPass(false);
    }
    if (elemIndicator < vecLength) {
        m_selection = selection;
        positionButtonsInside(elemIndicator);
    }
}

void ButtonHandler::updateBlockedSides()
{
    QRegion screenRegion{};
    for (const QRect& rect : m_screenRegions) {
        screenRegion += rect;
    }

    const int EXTENSION = m_separator * 2 + m_buttonBaseSize;
    // Right
    QPoint pointA(m_selection.right() + EXTENSION, m_selection.bottom());
    QPoint pointB(pointA.x(), m_selection.top());
    m_blockedRight =
      !(screenRegion.contains(pointA) && screenRegion.contains(pointB));
    // Left
    pointA.setX(m_selection.left() - EXTENSION);
    pointB.setX(pointA.x());
    m_blockedLeft =
      !(screenRegion.contains(pointA) && screenRegion.contains(pointB));
    // Bottom
    pointA = QPoint(m_selection.left(), m_selection.bottom() + EXTENSION);
    pointB = QPoint(m_selection.right(), pointA.y());
    m_blockedBotton =
      !(screenRegion.contains(pointA) && screenRegion.contains(pointB));
    // Top
    pointA.setY(m_selection.top() - EXTENSION);
    pointB.setY(pointA.y());
    m_blockedTop =
      !(screenRegion.contains(pointA) && screenRegion.contains(pointB));
    // Auxiliary
    m_oneHorizontalBlocked =
      (!m_blockedRight && m_blockedLeft) || (m_blockedRight && !m_blockedLeft);
    m_horizontalyBlocked = (m_blockedRight && m_blockedLeft);
    m_allSidesBlocked =
      (m_blockedBotton && m_horizontalyBlocked && m_blockedTop);
}

void ButtonHandler::expandSelection()
{
    int& s = m_buttonExtendedSize;
    m_selection = m_selection + QMargins(s, s, s, s);
    m_selection = intersectWithAreas(m_selection);
}

void ButtonHandler::positionButtonsInside(int index)
{
    // Position the buttons in the botton-center of the main but inside of the
    // selection.
    QRect mainArea = m_selection;
    mainArea = intersectWithAreas(mainArea);
    const int buttonsPerRow = (mainArea.width()) / (m_buttonExtendedSize);
    if (buttonsPerRow == 0) {
        return;
    }
    QPoint center =
      QPoint(mainArea.center().x(), mainArea.bottom() - m_buttonExtendedSize);

    while (m_vectorButtons.size() > index) {
        int addCounter = buttonsPerRow;
        addCounter = qBound(0, addCounter, m_vectorButtons.size() - index);
        QVector<QPoint> positions = horizontalPoints(center, addCounter, true);
        moveButtonsToPoints(positions, index);
        center.setY(center.y() - m_buttonExtendedSize);
    }

    m_buttonsAreInside = true;
}

void ButtonHandler::ensureSelectionMinimumSize()
{
    // Detect if a side is smaller than a button in order to prevent collision
    // and redimension the base area to the base size of a single button per
    // side
    if (m_selection.width() < m_buttonBaseSize) {
        if (!m_blockedLeft) {
            m_selection.setX(m_selection.x() -
                             (m_buttonBaseSize - m_selection.width()) / 2);
        }
        m_selection.setWidth(m_buttonBaseSize);
    }
    if (m_selection.height() < m_buttonBaseSize) {
        if (!m_blockedTop) {
            m_selection.setY(m_selection.y() -
                             (m_buttonBaseSize - m_selection.height()) / 2);
        }
        m_selection.setHeight(m_buttonBaseSize);
    }
}

void ButtonHandler::moveButtonsToPoints(const QVector<QPoint>& points,
                                        int& index)
{
    for (const QPoint& p : points) {
        auto* button = m_vectorButtons[index];
        button->move(p);
        ++index;
    }
}

void ButtonHandler::adjustHorizontalCenter(QPoint& center)
{
    if (m_blockedLeft) {
        center.setX(center.x() + m_buttonExtendedSize / 2);
    } else if (m_blockedRight) {
        center.setX(center.x() - m_buttonExtendedSize / 2);
    }
}

// setButtons redefines the buttons of the button handler
void ButtonHandler::setButtons(const QVector<CaptureToolButton*>& v)
{
    if (v.isEmpty()) {
        return;
    }

    for (CaptureToolButton* b : m_vectorButtons) {
        delete (b);
    }
    m_vectorButtons = v;
    m_buttonBaseSize = GlobalValues::buttonBaseSize();
    m_buttonExtendedSize = m_buttonBaseSize + m_separator;
}

bool ButtonHandler::contains(const QPoint& p) const
{
    if (m_vectorButtons.isEmpty()) {
        return false;
    }
    QPoint first(m_vectorButtons.first()->pos());
    QPoint last(m_vectorButtons.last()->pos());
    bool firstIsTopLeft = (first.x() <= last.x() && first.y() <= last.y());
    QPoint topLeft = firstIsTopLeft ? first : last;
    QPoint bottonRight = firstIsTopLeft ? last : first;
    topLeft += QPoint(-m_separator, -m_separator);
    bottonRight += QPoint(m_buttonExtendedSize, m_buttonExtendedSize);
    QRegion r(QRect(topLeft, bottonRight).normalized());
    return r.contains(p);
}

void ButtonHandler::updateScreenRegions(const QVector<QRect>& rects)
{
    m_screenRegions = rects;
}

void ButtonHandler::updateScreenRegions(const QRect& rect)
{
    m_screenRegions = { rect };
}
