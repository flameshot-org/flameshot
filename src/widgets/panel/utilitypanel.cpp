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

#include "utilitypanel.h"
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

UtilityPanel::UtilityPanel(QWidget* parent)
  : QWidget(parent)
{
    initInternalPanel();
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setCursor(Qt::ArrowCursor);

    m_showAnimation = new QPropertyAnimation(m_internalPanel, "geometry", this);
    m_showAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_showAnimation->setDuration(300);

    m_hideAnimation = new QPropertyAnimation(m_internalPanel, "geometry", this);
    m_hideAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_hideAnimation->setDuration(300);

    connect(m_hideAnimation,
            &QPropertyAnimation::finished,
            m_internalPanel,
            &QWidget::hide);

#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_MAC64) ||          \
     defined(Q_OS_MACOS) || defined(Q_OS_MACX))
    move(0, 0);
#endif
}

QWidget* UtilityPanel::toolWidget() const
{
    return m_toolWidget;
}

void UtilityPanel::addToolWidget(QWidget* w)
{
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
    }
    if (w) {
        m_toolWidget = w;
        m_toolWidget->setSizePolicy(QSizePolicy::Ignored,
                                    QSizePolicy::Preferred);
        m_upLayout->addWidget(w);
    }
}

void UtilityPanel::clearToolWidget()
{
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
    }
}

void UtilityPanel::pushWidget(QWidget* w)
{
    m_layout->insertWidget(m_layout->count() - 1, w);
}

void UtilityPanel::show()
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_showAnimation->setStartValue(QRect(-width(), 0, 0, height()));
    m_showAnimation->setEndValue(QRect(0, 0, width(), height()));
    m_internalPanel->show();
    m_showAnimation->start();
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_MAC64) ||          \
     defined(Q_OS_MACOS) || defined(Q_OS_MACX))
    move(0, 0);
#endif
    QWidget::show();
}

void UtilityPanel::hide()
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_hideAnimation->setStartValue(QRect(0, 0, width(), height()));
    m_hideAnimation->setEndValue(QRect(-width(), 0, 0, height()));
    m_hideAnimation->start();
    m_internalPanel->hide();
    QWidget::hide();
}

void UtilityPanel::toggle()
{
    if (m_internalPanel->isHidden()) {
        show();
    } else {
        hide();
    }
}

void UtilityPanel::initInternalPanel()
{
    m_internalPanel = new QScrollArea(this);
    m_internalPanel->setAttribute(Qt::WA_NoMousePropagation);
    QWidget* widget = new QWidget();
    m_internalPanel->setWidget(widget);
    m_internalPanel->setWidgetResizable(true);

    m_layout = new QVBoxLayout();
    m_upLayout = new QVBoxLayout();
    m_bottomLayout = new QVBoxLayout();
    m_layout->addLayout(m_upLayout);
    m_layout->addLayout(m_bottomLayout);
    widget->setLayout(m_layout);

    QPushButton* closeButton = new QPushButton(this);
    closeButton->setText(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &UtilityPanel::toggle);
    m_bottomLayout->addWidget(closeButton);

    QColor bgColor = palette().window().color();
    bgColor.setAlphaF(0.0);
    m_internalPanel->setStyleSheet(
      QStringLiteral("QScrollArea {background-color: %1}").arg(bgColor.name()));
    m_internalPanel->hide();
}
