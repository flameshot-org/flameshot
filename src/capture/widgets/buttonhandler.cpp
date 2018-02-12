// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "buttonhandler.h"
#include <QPoint>
#include <QGuiApplication>
#include <QScreen>

// ButtonHandler is a habdler for every active button. It makes easier to
// manipulate the buttons as a unit.

ButtonHandler::ButtonHandler(const QVector<CaptureButton*> &v,
                             QObject *parent) :
    QObject(parent)
{
    setButtons(v);
    init();
}

ButtonHandler::ButtonHandler(QObject *parent) :
    QObject(parent)
{
    init();
}

void ButtonHandler::hide() {
    for (CaptureButton *b: m_vectorButtons)
        b->hide();
}

void ButtonHandler::show() {
    if (m_vectorButtons.isEmpty() || m_vectorButtons.first()->isVisible()) {
        return;
    }
    for (CaptureButton *b: m_vectorButtons)
        b->animatedShow();
}

bool ButtonHandler::isVisible() const {
    bool ret = true;
    for (const CaptureButton *b: m_vectorButtons) {
        if (!b->isVisible()) {
            ret = false;
            break;
        }
    }
    return ret;
}

bool ButtonHandler::buttonsAreInside() const {
    return m_buttonsAreInside;
}

size_t ButtonHandler::size() const {
    return m_vectorButtons.size();
}

// updatePosition updates the position of the buttons around the
// selection area. Ignores the sides blocked by the end of the screen.
// When the selection is too small it works on a virtual selection with
// the original in the center.
void ButtonHandler::updatePosition(const QRect &selection) {
    resetRegionTrack();
    const int vecLength = m_vectorButtons.size();
    if (vecLength == 0) {
        return;
    }
    // Copy of the selection area for internal modifications
    m_selection = selection;
    updateBlockedSides();
    ensureSelectionMinimunSize();
    // Indicates the actual button to be moved
    int elemIndicator = 0;

    while (elemIndicator < vecLength) {

        // Add them inside the area when there is no more space
        if (m_allSidesBlocked) {
            positionButtonsInside(elemIndicator);
            break; // the while
        }
        // Number of buttons per row column
        int buttonsPerRow = (m_selection.width() + m_separator) / (m_buttonExtendedSize);
        int buttonsPerCol = (m_selection.height() + m_separator) / (m_buttonExtendedSize);
        // Buttons to be placed in the corners
        int extraButtons = (vecLength - elemIndicator) -
                (buttonsPerRow + buttonsPerCol) * 2;
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

        // Add buttons at the botton of the seletion
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
            QVector<QPoint> positions = horizontalPoints(center, addCounter, true);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // Add buttons at the right side of the seletion
        if (!m_blockedRight && elemIndicator < vecLength) {
            int addCounter = buttonsPerCol;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);

            QPoint center = QPoint(m_selection.right() + m_separator,
                                   m_selection.center().y());
            QVector<QPoint> positions = verticalPoints(center, addCounter, false);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // Add buttons at the top of the seletion
        if (!m_blockedTop && elemIndicator < vecLength) {
            int addCounter = buttonsPerRow + elemCornersTop;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);
            QPoint center = QPoint(m_selection.center().x(),
                                   m_selection.top() - m_buttonExtendedSize);
            if (addCounter == 1 + buttonsPerRow) {
                adjustHorizontalCenter(center);
            }
            QVector<QPoint> positions = horizontalPoints(center, addCounter, false);
            moveButtonsToPoints(positions, elemIndicator);
        }
        // Add buttons at the left side of the seletion
        if (!m_blockedLeft && elemIndicator < vecLength) {
            int addCounter = buttonsPerCol;
            addCounter = qBound(0, addCounter, vecLength - elemIndicator);

            QPoint center = QPoint(m_selection.left() - m_buttonExtendedSize,
                                   m_selection.center().y());
            QVector<QPoint> positions = verticalPoints(center, addCounter, true);
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

// horizontalPoints is an auxiliar method for the button position computation.
// starts from a known center and keeps adding elements horizontally
// and returns the computed positions.
QVector<QPoint> ButtonHandler::horizontalPoints(
        const QPoint &center, const int elements, const bool leftToRight) const
{
    QVector<QPoint> res;
    // Distance from the center to start adding buttons
    int shift = 0;
    if (elements % 2 == 0) {
        shift = m_buttonExtendedSize * (elements / 2) - (m_separator / 2);
    } else {
        shift = m_buttonExtendedSize * ((elements-1) / 2) + m_buttonBaseSize / 2;
    }
    if (!leftToRight) { shift -= m_buttonBaseSize; }
    int x = leftToRight ? center.x() - shift :
                          center.x() + shift;
    QPoint i(x, center.y());
    while (elements > res.length()) {
        res.append(i);
        leftToRight ? i.setX(i.x() + m_buttonExtendedSize) :
                      i.setX(i.x() - m_buttonExtendedSize);
    }
    return res;
}

// verticalPoints is an auxiliar method for the button position computation.
// starts from a known center and keeps adding elements vertically
// and returns the computed positions.
QVector<QPoint> ButtonHandler::verticalPoints(
        const QPoint &center, const int elements, const bool upToDown) const
{
    QVector<QPoint> res;
    // Distance from the center to start adding buttons
    int shift = 0;
    if (elements % 2 == 0) {
        shift = m_buttonExtendedSize * (elements / 2) - (m_separator / 2);
    } else {
        shift = m_buttonExtendedSize * ((elements-1) / 2) + m_buttonBaseSize / 2;
    }
    if (!upToDown) { shift -= m_buttonBaseSize; }
    int y = upToDown ? center.y() - shift :
                       center.y() + shift;
    QPoint i(center.x(), y);
    while (elements > res.length()) {
        res.append(i);
        upToDown ? i.setY(i.y() + m_buttonExtendedSize) :
                      i.setY(i.y() - m_buttonExtendedSize);
    }
    return res;
}

void ButtonHandler::init() {
    m_separator = CaptureButton::buttonBaseSize() / 4;
}

void ButtonHandler::resetRegionTrack() {
    m_buttonsAreInside = false;
}

void ButtonHandler::updateBlockedSides() {
    const int EXTENSION = m_separator * 2 + m_buttonBaseSize;
    // Right
    QPoint pointA(m_selection.right() + EXTENSION,
                m_selection.bottom());
    QPoint pointB(pointA.x(),
                m_selection.top());
    m_blockedRight = !(m_screenRegions.contains(pointA) &&
                       m_screenRegions.contains(pointB));
    // Left
    pointA.setX(m_selection.left() - EXTENSION);
    pointB.setX(pointA.x());
    m_blockedLeft = !(m_screenRegions.contains(pointA) &&
                      m_screenRegions.contains(pointB));
    // Botton
    pointA = QPoint(m_selection.left(),
                    m_selection.bottom() + EXTENSION);
    pointB = QPoint(m_selection.right(),
                    pointA.y());
    m_blockedBotton = !(m_screenRegions.contains(pointA) &&
                        m_screenRegions.contains(pointB));
    // Top
    pointA.setY(m_selection.top() - EXTENSION);
    pointB.setY(pointA.y());
    m_blockedTop = !(m_screenRegions.contains(pointA) &&
                     m_screenRegions.contains(pointB));
    // Auxiliar
    m_oneHorizontalBlocked = (!m_blockedRight && m_blockedLeft) ||
            (m_blockedRight && !m_blockedLeft);
    m_horizontalyBlocked = (m_blockedRight && m_blockedLeft);
    m_allSidesBlocked = (m_blockedBotton && m_horizontalyBlocked && m_blockedTop);
}

void ButtonHandler::expandSelection() {
    if (m_blockedRight && !m_blockedLeft) {
        m_selection.setX(m_selection.x() - m_buttonExtendedSize);
    } else if (!m_blockedRight && !m_blockedLeft) {
        m_selection.setX(m_selection.x() - m_buttonExtendedSize);
        m_selection.setWidth(m_selection.width() + m_buttonExtendedSize);
    } else {
        m_selection.setWidth(m_selection.width() + m_buttonExtendedSize);
    }

    if (m_blockedBotton && !m_blockedTop) {
        m_selection.setY(m_selection.y() - m_buttonExtendedSize);
    } else if (!m_blockedTop && !m_blockedBotton) {
        m_selection.setY(m_selection.y() - m_buttonExtendedSize);
        m_selection.setHeight(m_selection.height() + m_buttonExtendedSize);
    } else {
        m_selection.setHeight(m_selection.height() + m_buttonExtendedSize);
    }
}

void ButtonHandler::positionButtonsInside(int index) {
    // Position the buttons from left to right starting at the botton
    // left of the selection.
    // The main screen has priority as the reference when its x,y botton
    // left corner values are lower than the ones  of the selection.
    QRect mainArea = QGuiApplication::primaryScreen()->geometry();
    int xPos = m_selection.left() + m_separator;
    int yPos = m_selection.bottom() - m_buttonExtendedSize;
    if (m_selection.left() < mainArea.left()) {
        xPos = mainArea.left() + m_separator;
    }
    if (m_selection.bottom() > mainArea.bottom()) {
        yPos = mainArea.bottom() - m_buttonExtendedSize;
    }
    CaptureButton *button = nullptr;
    for (; index < m_vectorButtons.size(); ++index) {
        button = m_vectorButtons[index];
        button->move(xPos, yPos);
        if (button->pos().x() + m_buttonExtendedSize > mainArea.right()) {
            xPos = m_selection.left() + m_separator;
            yPos -= (m_buttonExtendedSize);
        }
        xPos += (m_buttonExtendedSize);
    }
    m_buttonsAreInside = true;
}

void ButtonHandler::ensureSelectionMinimunSize() {
    // Detect if a side is smaller than a button in order to prevent collision
    // and redimension the base area the the base size of a single button per side
    if (m_selection.width() < m_buttonBaseSize) {
        if (!m_blockedLeft) {
            m_selection.setX(m_selection.x() -
                             (m_buttonBaseSize-m_selection.width()) / 2);
        }
        m_selection.setWidth(m_buttonBaseSize);
    }
    if (m_selection.height() < m_buttonBaseSize) {
        if (!m_blockedTop) {
            m_selection.setY(m_selection.y() -
                             (m_buttonBaseSize-m_selection.height()) / 2);
        }
        m_selection.setHeight(m_buttonBaseSize);
    }
}

void ButtonHandler::moveButtonsToPoints(
        const QVector<QPoint> &points, int &index)
{
    for (const QPoint &p: points) {
        auto button = m_vectorButtons[index];
        button->move(p);
        ++index;
    }
}

void ButtonHandler::adjustHorizontalCenter(QPoint &center) {
    if (m_blockedLeft) {
        center.setX(center.x() + m_buttonExtendedSize/2);
    } else if (m_blockedRight) {
        center.setX(center.x() - m_buttonExtendedSize/2);
    }
}

// setButtons redefines the buttons of the button handler
void ButtonHandler::setButtons(const QVector<CaptureButton *> v) {
    if (v.isEmpty())
        return;

    for (CaptureButton *b: m_vectorButtons)
        delete(b);
    m_vectorButtons = v;
    m_buttonBaseSize = CaptureButton::buttonBaseSize();
    m_buttonExtendedSize = m_buttonBaseSize + m_separator;
}

bool ButtonHandler::contains(const QPoint &p) const {
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

void ButtonHandler::updateScreenRegions(const QVector<QRect> &rects) {
    m_screenRegions = QRegion();
    for (const QRect &rect: rects) {
        m_screenRegions += rect;
    }
}
