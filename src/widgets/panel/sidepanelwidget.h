// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#pragma once

#include <QWidget>
#include "color_wheel.hpp"

class QVBoxLayout;
class QPushButton;
class QLabel;
class QColorPickingEventFilter;
class QSlider;

class SidePanelWidget : public QWidget {
    Q_OBJECT

    friend class QColorPickingEventFilter;
public:
    explicit SidePanelWidget(QPixmap *p, QWidget *parent = nullptr);

signals:
    void colorChanged(const QColor &c);
    void thicknessChanged(const int &t);
    void togglePanel();

public slots:
    void updateColor(const QColor &c);
    void updateThickness(const int &t);

private slots:
    void updateColorNoWheel(const QColor &c);
    void updateCurrentThickness();

private slots:
    void colorGrabberActivated();
    void releaseColorGrab();

private:
    QColor grabPixmapColor(const QPoint &p);

    bool handleKeyPress(QKeyEvent *e);
    bool handleMouseButtonPressed(QMouseEvent *e);
    bool handleMouseMove(QMouseEvent *e);

    void updateGrabButton(const bool activated);

    QVBoxLayout *m_layout;
    QPushButton *m_colorGrabButton;
    color_widgets::ColorWheel *m_colorWheel;
    QLabel *m_colorLabel;
    QPixmap *m_pixmap;
    QColor m_colorBackup;
    QColor m_color;
    QSlider *m_thicknessSlider;
    int m_thickness;
    QColorPickingEventFilter *m_eventFilter;

};
