// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "sidepanelwidget.h"
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

class QColorPickingEventFilter : public QObject
{
public:
    explicit QColorPickingEventFilter(SidePanelWidget* pw,
                                      QObject* parent = nullptr)
      : QObject(parent)
      , m_pw(pw)
    {}

    bool eventFilter(QObject*, QEvent* event) override
    {
        event->accept();
        switch (event->type()) {
            case QEvent::MouseMove:
                return m_pw->handleMouseMove(static_cast<QMouseEvent*>(event));
            case QEvent::MouseButtonPress:
                return m_pw->handleMouseButtonPressed(
                  static_cast<QMouseEvent*>(event));
            case QEvent::KeyPress:
                return m_pw->handleKeyPress(static_cast<QKeyEvent*>(event));
            default:
                break;
        }
        return false;
    }

private:
    SidePanelWidget* m_pw;
};

////////////////////////

SidePanelWidget::SidePanelWidget(QPixmap* p, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(p)
  , m_eventFilter(nullptr)
{
    m_layout = new QVBoxLayout(this);

    QFormLayout* colorForm = new QFormLayout();
    m_thicknessSlider = new QSlider(Qt::Horizontal);
    m_thicknessSlider->setValue(m_thickness);
    m_colorLabel = new QLabel();
    m_colorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    colorForm->addRow(tr("Active thickness:"), m_thicknessSlider);
    colorForm->addRow(tr("Active color:"), m_colorLabel);
    m_layout->addLayout(colorForm);

    connect(m_thicknessSlider,
            &QSlider::valueChanged,
            this,
            &SidePanelWidget::updateCurrentThickness);
    connect(this,
            &SidePanelWidget::thicknessChanged,
            this,
            &SidePanelWidget::updateThickness);

    QColor background = this->palette().window().color();
    bool isDark = ColorUtils::colorIsDark(background);
    QString modifier =
      isDark ? PathInfo::whiteIconPath() : PathInfo::blackIconPath();
    QIcon grabIcon(modifier + "colorize.svg");
    m_colorGrabButton = new QPushButton(grabIcon, QLatin1String(""));
    updateGrabButton(false);
    connect(m_colorGrabButton,
            &QPushButton::pressed,
            this,
            &SidePanelWidget::colorGrabberActivated);
    m_layout->addWidget(m_colorGrabButton);

    m_colorWheel = new color_widgets::ColorWheel(this);
    m_colorWheel->setColor(m_color);
    connect(m_colorWheel,
            &color_widgets::ColorWheel::mouseReleaseOnColor,
            this,
            &SidePanelWidget::colorChanged);
    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorChanged,
            this,
            &SidePanelWidget::updateColorNoWheel);
    m_layout->addWidget(m_colorWheel);
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

void SidePanelWidget::colorGrabberActivated()
{
    grabKeyboard();
    grabMouse(Qt::CrossCursor);
    setMouseTracking(true);
    m_colorBackup = m_color;
    if (!m_eventFilter) {
        m_eventFilter = new QColorPickingEventFilter(this, this);
    }
    installEventFilter(m_eventFilter);
    updateGrabButton(true);
}

void SidePanelWidget::releaseColorGrab()
{
    setMouseTracking(false);
    removeEventFilter(m_eventFilter);
    releaseMouse();
    releaseKeyboard();
    setFocus();
    updateGrabButton(false);
}

QColor SidePanelWidget::grabPixmapColor(const QPoint& p)
{
    QColor c;
    if (m_pixmap) {
#if defined(Q_OS_MACOS)
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        QPoint point = p;
        if (currentScreen) {
            point = QPoint((p.x() - currentScreen->geometry().x()) *
                             currentScreen->devicePixelRatio(),
                           (p.y() - currentScreen->geometry().y()) *
                             currentScreen->devicePixelRatio());
        }
        QPixmap pixel = m_pixmap->copy(QRect(point, point));
#else
        QPixmap pixel = m_pixmap->copy(QRect(p, p));
#endif
        c = pixel.toImage().pixel(0, 0);
    }
    return c;
}

bool SidePanelWidget::handleKeyPress(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        releaseColorGrab();
        updateColor(m_colorBackup);
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        updateColor(grabPixmapColor(QCursor::pos()));
        releaseColorGrab();
        emit colorChanged(m_color);
    }
    return true;
}

bool SidePanelWidget::handleMouseButtonPressed(QMouseEvent* e)
{
    if (m_colorGrabButton->geometry().contains(e->pos()) ||
        e->button() == Qt::RightButton) {
        updateColorNoWheel(m_colorBackup);
    } else if (e->button() == Qt::LeftButton) {
        updateColor(grabPixmapColor(QCursor::pos()));
    }
    releaseColorGrab();
    emit colorChanged(m_color);
    return true;
}

bool SidePanelWidget::handleMouseMove(QMouseEvent* e)
{
    updateColorNoWheel(grabPixmapColor(e->globalPos()));
    return true;
}

void SidePanelWidget::updateGrabButton(const bool activated)
{
    if (activated) {
        m_colorGrabButton->setText(tr("Press ESC to cancel"));
    } else {
        m_colorGrabButton->setText(tr("Grab Color"));
    }
}
