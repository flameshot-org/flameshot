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

#include "notifierbox.h"
#include "src/utils/confighandler.h"
#include "src/capture/widgets/capturebutton.h"
#include <QTimer>
#include <QPainter>
#include <QApplication>

NotifierBox::NotifierBox(QWidget *parent) : QWidget(parent) {
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1200);
    connect(m_timer, &QTimer::timeout, this, &NotifierBox::hide);
    m_bgColor = ConfigHandler().uiMainColorValue();
    m_foregroundColor = (CaptureButton::iconIsWhiteByColor(m_bgColor) ?
                             Qt::white : Qt::black);
    m_bgColor.setAlpha(180);
    const int size = (CaptureButton::buttonBaseSize() +
                      CaptureButton::buttonBaseSize()/2) *
            qApp->devicePixelRatio();
    setFixedSize(QSize(size, size));
}

void NotifierBox::enterEvent(QEvent *) {
    hide();
}

void NotifierBox::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    // draw Elipse
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_bgColor, Qt::SolidPattern));
    painter.setPen(QPen(Qt::transparent));
    painter.drawEllipse(rect());
    // Draw the text:
    painter.setPen(QPen(m_foregroundColor));
    painter.drawText(rect(), Qt::AlignCenter, m_message);
}

void NotifierBox::showMessage(const QString &msg) {
    m_message = msg;
    update();
    show();
    m_timer->start();
}
