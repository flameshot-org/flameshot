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

#include "loadspinner.h"
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

#define OFFSET 5

LoadSpinner::LoadSpinner(QWidget *parent) :
    QWidget(parent), m_startAngle(0),  m_span(0), m_growing(true)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(100, 100);
    updateFrame();
    // init timer
    m_timer =  new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(rotate()));
    m_timer->setInterval(30);
}

void LoadSpinner::setColor(const QColor &c) {
    m_color = c;
}

void LoadSpinner::setWidth(int w) {
    setFixedSize(w, w);
    updateFrame();
}

void LoadSpinner::setHeight(int h) {
    setFixedSize(h, h);
    updateFrame();
}

void LoadSpinner::start() {
    m_timer->start();
}

void LoadSpinner::stop() {
    m_timer->stop();
}

void LoadSpinner::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    auto pen = QPen(m_color);

    pen.setWidth(5);
    painter.setPen(pen);
    painter.setOpacity(0.2);
    painter.drawArc(m_frame, 0,  5760);
    painter.setOpacity(1.0);
    painter.drawArc(m_frame, (m_startAngle * 16), (m_span * 16));

}

void LoadSpinner::rotate() {
    const int advance = 3;
    const int grow = 8;
    if (m_growing) {
        m_startAngle = (m_startAngle + advance) % 360;
        m_span += grow;
        if(m_span > 260) {
            m_growing = false;
        }
    } else {
        m_startAngle = (m_startAngle + grow) % 360;
        m_span = m_span + advance - grow;
        if(m_span < 10) {
            m_growing = true;
        }
    }
    update();
}

void LoadSpinner::updateFrame() {
    m_frame = QRect(OFFSET, OFFSET, width() - OFFSET*2, height() - OFFSET*2);
}
