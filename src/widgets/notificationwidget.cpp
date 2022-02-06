// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "notificationwidget.h"
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

NotificationWidget::NotificationWidget(QWidget* parent)
  : QWidget(parent)
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
    connect(
      m_hideAnimation, &QPropertyAnimation::finished, m_label, &QLabel::hide);

    auto* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    mainLayout->addWidget(m_content);
    m_layout->addWidget(m_label, 0, Qt::AlignHCenter);
    m_content->setLayout(m_layout);

    setFixedHeight(40);
}

void NotificationWidget::showMessage(const QString& msg)
{
    m_label->setText(msg);
    m_label->show();
    animatedShow();
}

void NotificationWidget::animatedShow()
{
    m_showAnimation->setStartValue(QRect(0, 0, width(), 0));
    m_showAnimation->setEndValue(QRect(0, 0, width(), height()));
    m_showAnimation->start();
    m_timer->start();
}

void NotificationWidget::animatedHide()
{
    m_hideAnimation->setStartValue(QRect(0, 0, width(), height()));
    m_hideAnimation->setEndValue(QRect(0, 0, width(), 0));
    m_hideAnimation->start();
}
