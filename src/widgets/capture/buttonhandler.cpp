// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "buttonhandler.h"
#include "utils/globalvalues.h"

#include <QPoint>
#include <QScreen>

#include <algorithm>
#include <numeric>

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
void ButtonHandler::updatePosition(const QRect& selection, const QPoint& anchor)
{
    resetRegionTrack();
    const int vecLength = m_vectorButtons.size();
    if (vecLength == 0) {
        return;
    }
    // Copy of the selection area for internal modifications
    m_selection = intersectWithAreas(selection);
    updateBlockedSides();
    ensureSelectionMinimumSize();
    // Indicates the actual button to be moved
    int elemIndicator = 0;
    // Collect the available slots before assigning buttons. This keeps the
    // existing screen-edge layout while allowing primary actions to follow
    // the pointer.
    QVector<QPoint> buttonPositions;
    buttonPositions.reserve(vecLength);

    while (elemIndicator < vecLength) {

        // Add them inside the area when there is no more space
        if (m_allSidesBlocked) {
            m_selection = selection;
            positionButtonsInside(elemIndicator, buttonPositions);
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
            const QVector<QPoint> sidePositions =
              horizontalPoints(center, addCounter, true);
            appendButtonPositions(
              sidePositions, buttonPositions, elemIndicator);
        }
        // Add buttons to the right side of the selection
        if (!m_blockedRight && elemIndicator < vecLength) {
            int addCounter = buttonsPerCol;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);

            QPoint center = QPoint(m_selection.right() + m_separator,
                                   m_selection.center().y());
            const QVector<QPoint> sidePositions =
              verticalPoints(center, addCounter, false);
            appendButtonPositions(
              sidePositions, buttonPositions, elemIndicator);
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
            const QVector<QPoint> sidePositions =
              horizontalPoints(center, addCounter, false);
            appendButtonPositions(
              sidePositions, buttonPositions, elemIndicator);
        }
        // Add buttons to the left side of the selection
        if (!m_blockedLeft && elemIndicator < vecLength) {
            int addCounter = buttonsPerCol;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);

            QPoint center = QPoint(m_selection.left() - m_buttonExtendedSize,
                                   m_selection.center().y());
            const QVector<QPoint> sidePositions =
              verticalPoints(center, addCounter, true);
            appendButtonPositions(
              sidePositions, buttonPositions, elemIndicator);
        }
        // If there are elements for the next cycle, increase the size of the
        // base area
        if (elemIndicator < vecLength && !(m_allSidesBlocked)) {
            expandSelection();
        }
        updateBlockedSides();
    }

    assignButtonsToPositions(buttonPositions, anchor);
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

void ButtonHandler::positionButtonsInside(int index, QVector<QPoint>& positions)
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
        const QVector<QPoint> rowPositions =
          horizontalPoints(center, addCounter, true);
        appendButtonPositions(rowPositions, positions, index);
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

void ButtonHandler::appendButtonPositions(const QVector<QPoint>& points,
                                          QVector<QPoint>& positions,
                                          int& index)
{
    for (const QPoint& p : points) {
        positions.append(p);
        ++index;
    }
}

void ButtonHandler::assignButtonsToPositions(const QVector<QPoint>& positions,
                                             const QPoint& anchor)
{
    if (positions.isEmpty()) {
        return;
    }

    QVector<CaptureToolButton*> primaryButtons;
    QVector<CaptureToolButton*> remainingButtons;
    primaryButtons.reserve(m_vectorButtons.size());
    remainingButtons.reserve(m_vectorButtons.size());

    const auto primaryRank = [](const CaptureToolButton* button) {
        switch (button->tool()->type()) {
            case CaptureTool::TYPE_COPY:
            case CaptureTool::TYPE_ACCEPT:
                return 0;
            case CaptureTool::TYPE_SAVE:
                return 1;
            case CaptureTool::TYPE_PIN:
                return 2;
            case CaptureTool::TYPE_PLUGIN:
                return 3;
            default:
                return -1;
        }
    };

    for (CaptureToolButton* button : m_vectorButtons) {
        if (primaryRank(button) >= 0) {
            primaryButtons.append(button);
        } else {
            remainingButtons.append(button);
        }
    }
    std::stable_sort(primaryButtons.begin(),
                     primaryButtons.end(),
                     [&primaryRank](const CaptureToolButton* left,
                                    const CaptureToolButton* right) {
                         return primaryRank(left) < primaryRank(right);
                     });

    // The first primary action gets the slot whose center is closest to the
    // release point, followed by the other primary actions.
    QVector<int> positionsByDistance(positions.size());
    std::iota(positionsByDistance.begin(), positionsByDistance.end(), 0);
    const QPoint buttonCenterOffset(m_buttonBaseSize / 2, m_buttonBaseSize / 2);
    const auto distanceSquared = [&](int positionIndex) {
        const QPoint delta =
          positions[positionIndex] + buttonCenterOffset - anchor;
        return static_cast<qint64>(delta.x()) * delta.x() +
               static_cast<qint64>(delta.y()) * delta.y();
    };
    std::stable_sort(positionsByDistance.begin(),
                     positionsByDistance.end(),
                     [&distanceSquared](int left, int right) {
                         return distanceSquared(left) < distanceSquared(right);
                     });

    QVector<bool> usedPositions(positions.size(), false);
    int primaryIndex = 0;
    for (; primaryIndex < primaryButtons.size() &&
           primaryIndex < positionsByDistance.size();
         ++primaryIndex) {
        const int positionIndex = positionsByDistance[primaryIndex];
        primaryButtons[primaryIndex]->move(positions[positionIndex]);
        usedPositions[positionIndex] = true;
    }

    int remainingIndex = 0;
    for (int positionIndex = 0; positionIndex < positions.size() &&
                                remainingIndex < remainingButtons.size();
         ++positionIndex) {
        if (!usedPositions[positionIndex]) {
            remainingButtons[remainingIndex]->move(positions[positionIndex]);
            ++remainingIndex;
        }
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

    QRegion buttonRegion;
    const QSize hitArea(m_buttonExtendedSize, m_buttonExtendedSize);
    for (const CaptureToolButton* button : m_vectorButtons) {
        const QPoint topLeft =
          button->pos() - QPoint(m_separator, m_separator);
        buttonRegion += QRect(topLeft, hitArea);
    }
    return buttonRegion.contains(p);
}

void ButtonHandler::updateScreenRegions(const QVector<QRect>& rects)
{
    m_screenRegions = rects;
}

void ButtonHandler::updateScreenRegions(const QRect& rect)
{
    m_screenRegions = { rect };
}
