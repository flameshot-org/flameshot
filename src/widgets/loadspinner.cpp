// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "loadspinner.h"
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

#define OFFSET 5

LoadSpinner::LoadSpinner(QWidget* parent)
  : QWidget(parent)
  , m_span(0)
  , m_growing(true)
{
    setAttribute(Qt::WA_TranslucentBackground);
    const int size = QApplication::fontMetrics().height() * 8;
    setFixedSize(size, size);
    updateFrame();
    // init timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &LoadSpinner::rotate);
    m_timer->setInterval(30);
}

void LoadSpinner::setColor(const QColor& c)
{
    m_color = c;
}

void LoadSpinner::setWidth(int w)
{
    setFixedSize(w, w);
    updateFrame();
}

void LoadSpinner::setHeight(int h)
{
    setFixedSize(h, h);
    updateFrame();
}

void LoadSpinner::start()
{
    m_timer->start();
}

void LoadSpinner::stop()
{
    m_timer->stop();
}

void LoadSpinner::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    auto pen = QPen(m_color);

    pen.setWidth(height() / 10);
    painter.setPen(pen);
    painter.setOpacity(0.2);
    painter.drawArc(m_frame, 0, 5760);
    painter.setOpacity(1.0);
    painter.drawArc(m_frame, (m_startAngle * 16), (m_span * 16));
}

void LoadSpinner::rotate()
{
    const int advance = 3;
    const int grow = 8;
    if (m_growing) {
        m_startAngle = (m_startAngle + advance) % 360;
        m_span += grow;
        if (m_span > 260) {
            m_growing = false;
        }
    } else {
        m_startAngle = (m_startAngle + grow) % 360;
        m_span = m_span + advance - grow;
        if (m_span < 10) {
            m_growing = true;
        }
    }
    update();
}

void LoadSpinner::updateFrame()
{
    m_frame =
      QRect(OFFSET, OFFSET, width() - OFFSET * 2, height() - OFFSET * 2);
}
