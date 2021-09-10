// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "sidepanelwidget.h"
#include "colorgrabwidget.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include "utilitypanel.h"
#include <QApplication>
#include <QDebug> // TODO remove
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
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
    if (parent) {
        parent->installEventFilter(this);
    }

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
    m_colorHex = new QLineEdit(this);
    m_colorHex->setAlignment(Qt::AlignCenter);

    QColor background = this->palette().window().color();
    bool isDark = ColorUtils::colorIsDark(background);
    QString modifier =
      isDark ? PathInfo::whiteIconPath() : PathInfo::blackIconPath();
    QIcon grabIcon(modifier + "colorize.svg");
    m_colorGrabButton = new QPushButton(grabIcon, tr("Grab Color"));

    m_layout->addWidget(m_colorGrabButton);
    m_layout->addWidget(m_colorWheel);
    m_layout->addWidget(m_colorHex);

    // thickness sigslots
    connect(m_thicknessSlider,
            &QSlider::valueChanged,
            this,
            &SidePanelWidget::updateCurrentThickness);
    connect(this,
            &SidePanelWidget::thicknessChanged,
            this,
            &SidePanelWidget::updateThickness);
    // color hex editor sigslots
    connect(m_colorHex, &QLineEdit::editingFinished, this, [=]() {
        if (!QColor::isValidColor(m_colorHex->text())) {
            m_colorHex->setText(m_color.name(QColor::HexRgb));
        } else {
            updateColor(m_colorHex->text());
        }
    });
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

void SidePanelWidget::updateColorNoWheel(const QColor& c)
{
    m_colorLabel->setStyleSheet(
      QStringLiteral("QLabel { background-color : %1; }").arg(c.name()));
    m_colorHex->setText(c.name(QColor::HexRgb));
}

void SidePanelWidget::updateThickness(const int& t)
{
    m_thickness = qBound(0, t, 100);
    m_thicknessSlider->setValue(m_thickness);
}

void SidePanelWidget::updateCurrentThickness(int value)
{
    emit thicknessChanged(value);
}

void SidePanelWidget::startColorGrab()
{
    m_revertColor = m_color;
    m_colorGrabber = new ColorGrabWidget(m_pixmap);
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

    emit togglePanel();
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
}

void SidePanelWidget::onColorUpdated(const QColor& color)
{
    updateColorNoWheel(color);
}

void SidePanelWidget::finalizeGrab()
{
    emit togglePanel();
}

bool SidePanelWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        // Override Escape shortcut from CaptureWidget
        auto* e = static_cast<QKeyEvent*>(event);
        if (e->key() == Qt::Key_Escape && m_colorHex->hasFocus()) {
            m_colorHex->clearFocus();
            e->accept();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        // Clicks outside of the Color Hex editor
        m_colorHex->clearFocus();
    }
    return QWidget::eventFilter(obj, event);
}

void SidePanelWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    m_colorHex->clearFocus();
}
