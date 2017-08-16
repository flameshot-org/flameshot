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

#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include "capturebutton.h"
#include <QVector>
#include <QObject>

class CaptureButton;
class QRect;
class QPoint;

class ButtonHandler : public QObject {
    Q_OBJECT
public:
    ButtonHandler(const QVector<CaptureButton*>&, QObject *parent = nullptr);
    ButtonHandler(QObject *parent = nullptr);

    void hideSectionUnderMouse(const QPoint &p);

    bool isVisible() const;
    bool isPartiallyHidden() const;
    bool buttonsAreInside() const;
    size_t size() const;

    void updatePosition(const QRect &selection, const QRect &limits);
    void setButtons(const QVector<CaptureButton*>);
    bool contains(const QPoint &p) const;

public slots:
    void hide();
    void show();

private:
    QVector<QPoint> horizontalPoints(const QPoint &center, const int elements,
                               const bool leftToRight) const;
    QVector<QPoint> verticalPoints(const QPoint &center, const int elements,
                               const bool upToDown) const;

    QVector<CaptureButton*> m_vectorButtons;

    QVector<CaptureButton*> m_topButtons;
    QVector<CaptureButton*> m_bottonButtons;
    QVector<CaptureButton*> m_leftButtons;
    QVector<CaptureButton*> m_rightButtons;
    QVector<CaptureButton*> m_insideButtons;
    QVector<CaptureButton*> * m_hiddenButtonVector;

    QRegion m_topRegion;
    QRegion m_bottonRegion;
    QRegion m_leftRegion;
    QRegion m_rightRegion;
    QRegion m_insideRegion;

    bool m_isPartiallyHidden;

    enum side {
        TOP, LEFT, BOTTON, RIGHT, INSIDE, NONE
    };

    int m_distance;
    int m_buttonBaseSize;
    bool m_buttonsAreInside;

    // aux methods
    void addToRegion(const QVector<QPoint> &points, const side s);
    void resetRegionTrack();

};

#endif // BUTTONHANDLER_H
