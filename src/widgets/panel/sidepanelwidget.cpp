// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "sidepanelwidget.h"
#include "colorgrabwidget.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#if defined(Q_OS_MACOS)
#include <QScreen>
#endif

SidePanelWidget::SidePanelWidget(QPixmap* p, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(p)
{
    m_layout = new QVBoxLayout(this);

    QFormLayout* colorForm = new QFormLayout();
    m_thicknessSlider = new QSlider(Qt::Horizontal);
    m_thicknessSlider->setRange(1, 100);
    m_thicknessSlider->setValue(m_thickness);
    m_colorLabel = new QLabel();
    m_colorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    colorForm->addRow(tr("Active thickness:"), m_thicknessSlider);
    colorForm->addRow(tr("Active color:"), m_colorLabel);
    m_layout->addLayout(colorForm);

    m_colorWheel = new color_widgets::ColorWheel(this);
    m_colorWheel->setColor(m_color);

    QColor background = this->palette().window().color();
    bool isDark = ColorUtils::colorIsDark(background);
    QString modifier =
      isDark ? PathInfo::whiteIconPath() : PathInfo::blackIconPath();
    QIcon grabIcon(modifier + "colorize.svg");
    m_colorGrabButton = new QPushButton(grabIcon, QLatin1String(""));
    m_colorGrabber = new ColorGrabWidget(m_pixmap, this);
    updateGrabButton(false);
    m_layout->addWidget(m_colorGrabButton);
    m_layout->addWidget(m_colorWheel);

    // thickness sigslots
    connect(m_thicknessSlider,
            &QSlider::valueChanged,
            this,
            &SidePanelWidget::updateCurrentThickness);
    connect(this,
            &SidePanelWidget::thicknessChanged,
            this,
            &SidePanelWidget::updateThickness);
    // color grab sigslots
    connect(m_colorGrabButton,
            &QPushButton::pressed,
            this,
            &SidePanelWidget::startColorGrab);
    connect(m_colorGrabber,
            &ColorGrabWidget::colorUpdated,
            this,
            &SidePanelWidget::onColorUpdated);
    connect(m_colorGrabber,
            &ColorGrabWidget::colorGrabbed,
            this,
            &SidePanelWidget::onColorGrabFinished);
    connect(m_colorGrabber,
            &ColorGrabWidget::grabAborted,
            this,
            &SidePanelWidget::onColorGrabAborted);
    // color wheel sigslots
    connect(m_colorWheel,
            &color_widgets::ColorWheel::mouseReleaseOnColor,
            this,
            &SidePanelWidget::colorChanged);
    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorChanged,
            this,
            &SidePanelWidget::updateColorNoWheel);

}

void SidePanelWidget::updateColor(const QColor& c)
{
    m_color = c;
    m_colorLabel->setStyleSheet(
      QStringLiteral("QLabel { background-color : %1; }").arg(c.name()));
    m_colorWheel->setColor(m_color);
}

void SidePanelWidget::updateThickness(const int& t)
{
    m_thickness = qBound(0, t, 100);
    m_thicknessSlider->setValue(m_thickness);
}

void SidePanelWidget::updateColorNoWheel(const QColor& c)
{
    m_color = c;
    m_colorLabel->setStyleSheet(
      QStringLiteral("QLabel { background-color : %1; }").arg(c.name()));
}

void SidePanelWidget::updateCurrentThickness(int value)
{
    emit thicknessChanged(value);
}

void SidePanelWidget::startColorGrab()
{
    updateGrabButton(true);
    m_colorGrabber->startGrabbing();
}

void SidePanelWidget::onColorGrabFinished()
{
    setFocus();
    updateGrabButton(false);
    m_color = m_colorGrabber->color();
    emit colorChanged(m_color);
}

void SidePanelWidget::onColorGrabAborted()
{
    // Restore color that was selected before we started grabbing
    updateColor(m_color);
    updateGrabButton(false);
}

bool SidePanelWidget::handleMouseButtonPressed(QMouseEvent* e)
{
// TODO what is this
//    if (m_colorGrabButton->geometry().contains(e->pos()) ||
//        e->button() == Qt::RightButton)
//        updateColorNoWheel(m_color);
    return true;
}

void SidePanelWidget::onColorUpdated(const QColor& color)
{
    updateColorNoWheel(color);
}

void SidePanelWidget::updateGrabButton(const bool activated)
{
    if (activated) {
        m_colorGrabButton->setText(tr("Press ESC to cancel"));
    } else {
        m_colorGrabButton->setText(tr("Grab Color"));
    }
}
