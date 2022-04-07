// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "sidepanelwidget.h"
#include "colorgrabwidget.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include "utilitypanel.h"
#include <QApplication>
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
  , m_layout(new QVBoxLayout(this))
  , m_pixmap(p)
{

    if (parent != nullptr) {
        parent->installEventFilter(this);
    }

    auto* colorLayout = new QGridLayout();

    // Create Active Tool Size
    auto* activeToolSizeText = new QLabel(tr("Active tool size: "));

    m_toolSizeSlider = new QSlider(Qt::Horizontal);
    m_toolSizeSlider->setRange(1, maxToolSize);
    m_toolSizeSlider->setValue(m_toolSize);
    m_toolSizeSlider->setMinimumWidth(minSliderWidth);

    colorLayout->addWidget(activeToolSizeText, 0, 0);
    colorLayout->addWidget(m_toolSizeSlider, 1, 0);

    // Create Active Color
    auto* colorHBox = new QHBoxLayout();
    auto* colorText = new QLabel(tr("Active Color: "));

    m_colorLabel = new QLabel();
    m_colorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    colorHBox->addWidget(colorText);
    colorHBox->addWidget(m_colorLabel);
    colorLayout->addLayout(colorHBox, 2, 0);

    m_layout->addLayout(colorLayout);

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

    // tool size sigslots
    connect(m_toolSizeSlider,
            &QSlider::valueChanged,
            this,
            &SidePanelWidget::toolSizeChanged);
    connect(this,
            &SidePanelWidget::toolSizeChanged,
            this,
            &SidePanelWidget::onToolSizeChanged);
    // color hex editor sigslots
    connect(m_colorHex, &QLineEdit::editingFinished, this, [=]() {
        if (!QColor::isValidColor(m_colorHex->text())) {
            m_colorHex->setText(m_color.name(QColor::HexRgb));
        } else {
            emit colorChanged(m_colorHex->text());
        }
    });
    // color grab button sigslots
    connect(m_colorGrabButton,
            &QPushButton::pressed,
            this,
            &SidePanelWidget::startColorGrab);
    // color wheel sigslots
    //   re-emit ColorWheel::colorSelected as SidePanelWidget::colorChanged
    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorSelected,
            this,
            &SidePanelWidget::colorChanged);
}

void SidePanelWidget::onColorChanged(const QColor& color)
{
    m_color = color;
    updateColorNoWheel(color);
    m_colorWheel->setColor(color);
}

void SidePanelWidget::onToolSizeChanged(int t)
{
    m_toolSize = qBound(0, t, maxToolSize);
    m_toolSizeSlider->setValue(m_toolSize);
}

void SidePanelWidget::startColorGrab()
{
    m_revertColor = m_color;
    m_colorGrabber = new ColorGrabWidget(m_pixmap);
    connect(m_colorGrabber,
            &ColorGrabWidget::colorUpdated,
            this,
            &SidePanelWidget::onTemporaryColorUpdated);
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
    onColorChanged(m_revertColor);
}

void SidePanelWidget::onTemporaryColorUpdated(const QColor& color)
{
    updateColorNoWheel(color);
}

void SidePanelWidget::finalizeGrab()
{
    emit togglePanel();
}

void SidePanelWidget::updateColorNoWheel(const QColor& c)
{
    m_colorLabel->setStyleSheet(
      QStringLiteral("QLabel { background-color : %1; }").arg(c.name()));
    m_colorHex->setText(c.name(QColor::HexRgb));
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
