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

#pragma once

#include <QWidget>

class LoadSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit LoadSpinner(QWidget *parent = nullptr);

    void setColor(const QColor &c);
    void setWidth(int w);
    void setHeight(int h);
    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void rotate();

private:
    QColor m_color;
    QTimer *m_timer;

    int m_startAngle = 0;
    int m_span =180;
    bool m_growing;

    QRect  m_frame;
    void updateFrame();
};
