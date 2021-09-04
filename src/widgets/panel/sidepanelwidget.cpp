// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "sidepanelwidget.h"
#include "colorgrabwidget.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include <QApplication>
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
    // color grab button sigslots
    connect(m_colorGrabButton,
            &QPushButton::pressed,
            this,
            &SidePanelWidget::startColorGrab);
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
    updateColorNoWheel(c);
    m_colorWheel->setColor(c);
}

void SidePanelWidget::updateThickness(const int& t)
{
    m_thickness = qBound(0, t, 100);
    m_thicknessSlider->setValue(m_thickness);
}

void SidePanelWidget::updateColorNoWheel(const QColor& c)
{
    m_colorLabel->setStyleSheet(
      QStringLiteral("QLabel { background-color : %1; }").arg(c.name()));
}

void SidePanelWidget::updateCurrentThickness(int value)
{
    emit thicknessChanged(value);
}

void SidePanelWidget::startColorGrab()
{
    m_revertColor = m_color;
    m_colorGrabber = new ColorGrabWidget(m_pixmap);
    updateGrabButton(true);
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
    m_colorGrabber->startGrabbing();
}

void SidePanelWidget::onColorGrabFinished()
{
    finalizeGrab();
    m_color = m_colorGrabber->color();
    emit colorChanged(m_color);
}

void SidePanelWidget::onColorGrabAborted()
{
    finalizeGrab();
    // Restore color that was selected before we started grabbing
    updateColor(m_revertColor);
    updateGrabButton(false);
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

void SidePanelWidget::finalizeGrab()
{
    updateGrabButton(false);
    // Unhovers the button - a minor detail
    QEvent leaveEvent(QEvent::Leave);
    qApp->sendEvent(m_colorGrabButton, &leaveEvent);
}
