// Copyright 2017 Alejandro Sirgo Rica
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

// ButtonHandler is a habdler for every active button. It makes easier to
// manipulate the buttons as a unit.

namespace {
    const int SEPARATION = 6;
}

ButtonHandler::ButtonHandler(const QVector<CaptureButton*> &v, QObject *parent) :
    QObject(parent)
{
    if (!v.isEmpty()) {
        m_buttonBaseSize = v[0]->getButtonBaseSize();
        m_distance = m_buttonBaseSize + SEPARATION;
        m_vectorButtons = v;
    }
}

ButtonHandler::ButtonHandler(QObject *parent) :
    QObject(parent)
{
}

void ButtonHandler::hide() {
    for (CaptureButton *b: m_vectorButtons) b->hide();
}

void ButtonHandler::show() {
    for (CaptureButton *b: m_vectorButtons) b->animatedShow();
}

bool ButtonHandler::isVisible() const {
    bool ret = false;
    if (!m_vectorButtons.isEmpty()) ret = m_vectorButtons[0]->isVisible();
    return ret;
}

size_t ButtonHandler::size() const {
    return m_vectorButtons.size();
}
// updatePosition updates the position of the buttons arround the
// selection area. Ignores the sides blocked by the end of the screen.
// When the selection is too small it works on a virtual selection with
// the original in the center.
void ButtonHandler::updatePosition(const QRect &selection,
                                   const  QRect &limits)
{
    m_region = QRegion();
    const QVector<CaptureButton*>::size_type vecLength = m_vectorButtons.size();
    if (vecLength == 0) {
        return;
    }
    // button dimmensions
    const int baseHeight = m_buttonBaseSize;
    const int baseWidth = m_buttonBaseSize;
    // copy of the selection area for internal modifications
    QRect baseArea = selection;

    // calculates the blocked sides (no more space for buttons)
    bool blockedRight =
            (limits.right() - baseArea.right() < SEPARATION*2 + baseWidth);
    bool blockedLeft =
            (baseArea.x() < baseWidth + SEPARATION*2);
    bool blockedBotton =
            (limits.bottom() - baseArea.bottom() < SEPARATION*2 + baseHeight);
    bool blockedTop =
            (baseArea.y() < baseHeight + SEPARATION*2);

    // detect if a side is smaller than a button in order to prevent collision
    // and redimension the base area the the base size of a single button per side
    if (baseArea.width() < baseWidth) {
        if (blockedRight && !blockedLeft) {
            baseArea.setX(baseArea.x() - (baseWidth-baseArea.width()));
        } else if (!blockedLeft && !blockedRight) {
            // when not close to the left side (because the rect grows to the right)
            baseArea.setX(baseArea.x() - (baseWidth-baseArea.width()) / 2);
        }
        baseArea.setWidth(baseWidth);
    }
    if (baseArea.height() < baseHeight) {
        if (blockedBotton && !blockedTop) {
            baseArea.setY(baseArea.y() - (baseHeight-baseArea.height()));
        } else if (!blockedTop && !blockedBotton) {
            // when not close to the top (because the rect grows to the bottom)
            baseArea.setY(baseArea.y() - (baseHeight-baseArea.height()) / 2);
        }
        baseArea.setHeight(baseHeight);
    }
    // indicates the actual button to be moved
    QVector<CaptureButton*>::size_type elemIndicator = 0;

    while (elemIndicator < vecLength) {
        // update of blocked sides
        blockedRight = (limits.right() - baseArea.right() < SEPARATION * 2 + baseWidth);
        blockedLeft = (baseArea.x() < baseWidth + SEPARATION * 2);
        blockedBotton = (limits.bottom() - baseArea.bottom() < SEPARATION * 2 + baseHeight);
        blockedTop = (baseArea.y() < baseHeight + SEPARATION * 2);
        // helper booleans
        bool oneHorizontalBlocked = (!blockedRight && blockedLeft) ||
                (blockedRight && !blockedLeft);
        bool horizontalBlocked = blockedRight && blockedLeft;

        // add them inside the area when there is no more space
        if (blockedBotton && horizontalBlocked && blockedTop) {
            QVector<QPoint> positions;
            int buttonsPerRow = (baseArea.width() - SEPARATION) / (baseWidth + SEPARATION);
            int xPos = baseArea.left() + SEPARATION;
            int yPos = baseArea.bottom() - SEPARATION - baseHeight;
            positions << QPoint(xPos, yPos);
            for (; elemIndicator < vecLength; ++elemIndicator) {
                if (elemIndicator % buttonsPerRow == 0 && elemIndicator != 0) {
                    xPos = baseArea.left() + SEPARATION;
                    yPos -= (SEPARATION + baseHeight);
                }
                m_vectorButtons[elemIndicator]->move(xPos, yPos);
                xPos += (SEPARATION + baseWidth);
                positions << QPoint(xPos, yPos);
            }
            addToRegion(positions);
            break; // the while
        }
        // number of buttons per row column
        int buttonsPerRow = (baseArea.width() + SEPARATION) / (baseWidth + SEPARATION);
        int buttonsPerCol = (baseArea.height() + SEPARATION) / (baseHeight + SEPARATION);
        // buttons to be placed in the corners
        int extraButtons = vecLength - buttonsPerRow*2 - buttonsPerCol*2;
        int elemsAtCorners = extraButtons > 4 ? 4 : extraButtons;
        // add buttons at the botton of the seletion
        if (!blockedBotton) {
            int addCounter =
                    (vecLength - elemIndicator < buttonsPerRow) ?
                      vecLength - elemIndicator : buttonsPerRow;
            if (elemsAtCorners > 2) {
                int temp = elemsAtCorners - 2;
                if (oneHorizontalBlocked && temp > 1) {
                    temp -= 1;
                }
                addCounter += temp;
            }
            QPoint center = QPoint(baseArea.center().x(),
                                   baseArea.bottom() + SEPARATION);
            if (addCounter > buttonsPerRow) {
                if (blockedLeft) {
                    center.setX(center.x() + (baseWidth+SEPARATION)/2);
                } else if (blockedRight) {
                    center.setX(center.x() - (baseWidth+SEPARATION)/2);
                }
            }

            QVector<QPoint> positions = getHPoints(center, addCounter, true);
            for (QPoint p: positions) {
                m_vectorButtons[elemIndicator]->move(p);
                ++elemIndicator;
            }
            addToRegion(positions);
        }
        // add buttons at the right side of the seletion
        if (!blockedRight && elemIndicator < vecLength) {
            int addCounter =
                    (vecLength - elemIndicator < buttonsPerCol) ?
                      vecLength - elemIndicator : buttonsPerCol;

            QPoint center = QPoint(baseArea.right() + SEPARATION,
                                   baseArea.center().y());
            QVector<QPoint> positions = getVPoints(center, addCounter, false);
            for (QPoint p: positions) {
                m_vectorButtons[elemIndicator]->move(p);
                ++elemIndicator;
            }
            addToRegion(positions);
        }
        // add buttons at the top of the seletion
        if (!blockedTop && elemIndicator < vecLength) {
            int addCounter =
                    (vecLength - elemIndicator < buttonsPerRow) ?
                      vecLength - elemIndicator : buttonsPerRow;

            if (elemsAtCorners > 1 && !horizontalBlocked && !oneHorizontalBlocked) {
                addCounter += 2;
            } else if ((elemsAtCorners == 1 &&
                        (!horizontalBlocked || oneHorizontalBlocked)) ||
                        (elemsAtCorners > 1 && oneHorizontalBlocked)) {
                addCounter += 1;
            }
            QPoint center = QPoint(baseArea.center().x(),
                                   baseArea.top() - (SEPARATION+baseHeight));
            if (addCounter == 1 + buttonsPerRow) {
                if (blockedLeft) {
                    center.setX(center.x() + (baseWidth+SEPARATION)/2);
                } else if (blockedRight) {
                    center.setX(center.x() - (baseWidth+SEPARATION)/2);
                }
            }
            QVector<QPoint> positions = getHPoints(center, addCounter, false);
            for (QPoint p: positions) {
                m_vectorButtons[elemIndicator]->move(p);
                ++elemIndicator;
            }
            addToRegion(positions);
        }
        // add buttons at the left side of the seletion
        if (!blockedLeft && elemIndicator < vecLength) {
            int addCounter =
                    (vecLength - elemIndicator < buttonsPerCol) ?
                      vecLength - elemIndicator : buttonsPerCol;

            if (vecLength - elemIndicator < buttonsPerCol) {
                addCounter = vecLength - elemIndicator;
            }
            QPoint center = QPoint(baseArea.left() - (SEPARATION + baseWidth),
                                   baseArea.center().y());
            QVector<QPoint> positions = getVPoints(center, addCounter, true);
            for (QPoint p: positions) {
                m_vectorButtons[elemIndicator]->move(p);
                ++elemIndicator;
            }
            addToRegion(positions);
        }
        // if there are elements for the next cycle, increase the size of the base area
        if (elemIndicator < vecLength &&
                !(blockedBotton && horizontalBlocked && blockedTop)) {

            if (blockedRight && !blockedLeft) {
                baseArea.setX(baseArea.x() - (baseWidth + SEPARATION));
            } else if (!blockedRight && !blockedLeft) {
                baseArea.setX(baseArea.x() - (baseWidth + SEPARATION));
                baseArea.setWidth(baseArea.width() + (baseWidth + SEPARATION));
            } else {
                baseArea.setWidth(baseArea.width() + (baseWidth + SEPARATION));
            }

            if (blockedBotton && !blockedTop) {
                baseArea.setY(baseArea.y() - (baseHeight + SEPARATION));
            } else if (!blockedTop && !blockedBotton) {
                baseArea.setY(baseArea.y() - (baseHeight + SEPARATION));
                baseArea.setHeight(baseArea.height() + (baseHeight + SEPARATION));
            } else {
                baseArea.setHeight(baseArea.height() + (baseHeight + SEPARATION));
            }
        }
    }
}

// getHPoints is an auxiliar method for the button position computation.
// starts from a known center and keeps adding elements horizontally
// and returns the computed positions.
QVector<QPoint> ButtonHandler::getHPoints(
        const QPoint &center, const int elements, const bool leftToRight) const
{
    QVector<QPoint> res;
    // distance from the center to start adding buttons
    int shift = 0;
    if (elements % 2 == 0) {
        shift = m_distance * (elements / 2) - (SEPARATION / 2);
    } else {
        shift = m_distance * ((elements-1) / 2) + m_buttonBaseSize / 2;
    }
    if (!leftToRight) { shift -= m_buttonBaseSize; }
    int x = leftToRight ? center.x() - shift :
                          center.x() + shift;
    QPoint i(x, center.y());
    while (elements > res.length()) {
        res.append(i);
        leftToRight ? i.setX(i.x() + m_distance) :
                      i.setX(i.x() - m_distance);
    }
    return res;
}

// getHPoints is an auxiliar method for the button position computation.
// starts from a known center and keeps adding elements vertically
// and returns the computed positions.
QVector<QPoint> ButtonHandler::getVPoints(
        const QPoint &center, const int elements,const bool upToDown) const
{
    QVector<QPoint> res;
    // distance from the center to start adding buttons
    int shift = 0;
    if (elements % 2 == 0) {
        shift = m_distance * (elements / 2) - (SEPARATION / 2);
    } else {
        shift = m_distance * ((elements-1) / 2) + m_buttonBaseSize / 2;
    }
    if (!upToDown) { shift -= m_buttonBaseSize; }
    int y = upToDown ? center.y() - shift :
                       center.y() + shift;
    QPoint i(center.x(), y);
    while (elements > res.length()) {
        res.append(i);
        upToDown ? i.setY(i.y() + m_distance) :
                      i.setY(i.y() - m_distance);
    }
    return res;
}

void ButtonHandler::addToRegion(const QVector<QPoint> &points) {
    if (!points.isEmpty()) {
        QPoint first(points.first());
        QPoint last(points.last());
        bool firstIsTopLeft = (first.x() <= last.x() && first.y() <= last.y());
        QPoint topLeft = firstIsTopLeft ? first : last;
        QPoint bottonRight = firstIsTopLeft ? last : first;
        bottonRight += QPoint(m_buttonBaseSize, m_buttonBaseSize);
        m_region += QRegion(QRect(topLeft, bottonRight).normalized());
    }
}
// setButtons redefines the buttons of the button handler
void ButtonHandler::setButtons(const QVector<CaptureButton *> v) {
    for (CaptureButton *b: m_vectorButtons) delete(b);
    m_vectorButtons = v;
    if (!v.isEmpty()) {
        m_buttonBaseSize = v[0]->getButtonBaseSize();
        m_distance = m_buttonBaseSize + SEPARATION;
    }
}

bool ButtonHandler::contains(const QPoint &p) const {
    return m_region.contains(p);
}
