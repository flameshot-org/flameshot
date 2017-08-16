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

#include "notificationwidget.h"
#include <QLabel>
#include <QTimer>
#include <QLabel>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QFrame>
#include <QIcon>

NotificationWidget::NotificationWidget(QWidget *parent) : QWidget(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(7000);
    connect(m_timer, &QTimer::timeout, this, &NotificationWidget::animatedHide);

    m_content = new QFrame();
    m_layout = new QVBoxLayout();
    m_label = new QLabel(m_content);
    m_label->hide();

    m_showAnimation = new QPropertyAnimation(m_content, "geometry", this);
    m_showAnimation->setDuration(300);

    m_hideAnimation = new QPropertyAnimation(m_content, "geometry", this);
    m_hideAnimation->setDuration(300);
    connect(m_hideAnimation, &QPropertyAnimation::finished, m_label, &QLabel::hide);

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    mainLayout->addWidget(m_content);
    m_layout->addWidget(m_label, 0, Qt::AlignHCenter);
    m_content->setLayout(m_layout);

    setFixedHeight(40);
}

void NotificationWidget::showMessage(const QString &msg) {
    m_label->setText(msg);
    m_label->show();
    animatedShow();
}

void NotificationWidget::animatedShow() {
    m_showAnimation->setStartValue(QRect(0, 0, width(), 0));
    m_showAnimation->setEndValue(QRect(0, 0, width(), height()));
    m_showAnimation->start();
    m_timer->start();
}

void NotificationWidget::animatedHide() {
    m_hideAnimation->setStartValue(QRect(0, 0, width(), height()));
    m_hideAnimation->setEndValue(QRect(0, 0, width(), 0));
    m_hideAnimation->start();
}
