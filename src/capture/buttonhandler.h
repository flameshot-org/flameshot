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

    void hide();
    void show();

    bool isVisible() const;
    size_t size() const;

    void updatePosition(const QRect &selection, const QRect &limits);
    void setButtons(const QVector<CaptureButton*>);
    bool contains(const QPoint &p) const;

private:
    QVector<QPoint> getHPoints(const QPoint &center, const int elements,
                               const bool leftToRight) const;
    QVector<QPoint> getVPoints(const QPoint &center, const int elements,
                               const bool upToDown) const;
    QVector<CaptureButton*> m_vectorButtons;

    int m_distance;
    int m_buttonBaseSize;

    QRegion m_region;
    void addToRegion(const QVector<QPoint> &points);
};

#endif // BUTTONHANDLER_H
