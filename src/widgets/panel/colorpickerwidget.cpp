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

#include "colorpickerwidget.h"
#include "src/utils/pathinfo.h"
#include "src/utils/colorutils.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>

class QColorPickingEventFilter : public QObject {
public:

    explicit QColorPickingEventFilter(
            ColorPickerWidget *pw, QObject *parent = 0) :
        QObject(parent), m_pw(pw) {}

    bool eventFilter(QObject *, QEvent *event) override {
        event->accept();
        switch (event->type()) {
        case QEvent::MouseMove:
            return m_pw->handleMouseMove(static_cast<QMouseEvent *>(event));
        case QEvent::MouseButtonPress:
            return m_pw->handleMouseButtonPressed(
                        static_cast<QMouseEvent *>(event));
        case QEvent::KeyPress:
            return m_pw->handleKeyPress(static_cast<QKeyEvent *>(event));
        default:
            break;
        }
        return false;
    }

private:
    ColorPickerWidget *m_pw;
};

////////////////////////

ColorPickerWidget::ColorPickerWidget(QPixmap *p, QWidget *parent) :
    QWidget(parent), m_pixmap(p), m_eventFilter(nullptr)
{
    m_layout = new QVBoxLayout(this);

    QFormLayout *colorForm = new QFormLayout();
    m_colorLabel = new QLabel();
    m_colorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    colorForm->addRow(tr("Active color:"), m_colorLabel);
    m_layout->addLayout(colorForm);

    QColor background = this->palette().background().color();
    bool isDark = ColorUtils::colorIsDark(background);
    QString modifier = isDark ? PathInfo::whiteIconPath() :
                                PathInfo::blackIconPath();
    QIcon grabIcon(modifier + "colorize.svg");
    m_colorGrabButton = new QPushButton(grabIcon, "");
    updateGrabButton(false);
    connect(m_colorGrabButton, &QPushButton::pressed,
            this, &ColorPickerWidget::colorGrabberActivated);
    m_layout->addWidget(m_colorGrabButton);

    m_colorWheel = new color_widgets::ColorWheel(this);
    m_colorWheel->setColor(m_color);
    connect(m_colorWheel, &color_widgets::ColorWheel::mouseReleaseOnColor, this,
            &ColorPickerWidget::colorChanged);
    connect(m_colorWheel, &color_widgets::ColorWheel::colorChanged, this,
            &ColorPickerWidget::updateColorNoWheel);
    m_layout->addWidget(m_colorWheel);
}

void ColorPickerWidget::updateColor(const QColor &c) {
    m_color = c;
    m_colorLabel->setStyleSheet(
                QString("QLabel { background-color : %1; }").arg(c.name()));
    m_colorWheel->setColor(m_color);
}

void ColorPickerWidget::updateColorNoWheel(const QColor &c) {
    m_color = c;
    m_colorLabel->setStyleSheet(
                QString("QLabel { background-color : %1; }").arg(c.name()));
}

void ColorPickerWidget::colorGrabberActivated() {
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

void ColorPickerWidget::releaseColorGrab() {
    setMouseTracking(false);
    removeEventFilter(m_eventFilter);
    releaseMouse();
    releaseKeyboard();
    setFocus();
    updateGrabButton(false);
}

QColor ColorPickerWidget::grabPixmapColor(const QPoint &p) {
    QColor c;
    if (m_pixmap) {
        QPixmap pixel = m_pixmap->copy(QRect(p, p));
        c = pixel.toImage().pixel(0,0);
    }
    return c;
}

bool ColorPickerWidget::handleKeyPress(QKeyEvent *e) {
    if (e->key() == Qt::Key_Space) {
        emit togglePanel();
    } else if (e->key() == Qt::Key_Escape) {
        releaseColorGrab();
        updateColor(m_colorBackup);
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        updateColor(grabPixmapColor(QCursor::pos()));
        releaseColorGrab();
        emit colorChanged(m_color);
    }
    return true;
}

bool ColorPickerWidget::handleMouseButtonPressed(QMouseEvent *e) {
    if (m_colorGrabButton->geometry().contains(e->pos()) ||
            e->button() == Qt::RightButton)
    {
        updateColorNoWheel(m_colorBackup);
    } else if (e->button() == Qt::LeftButton) {
        updateColor(grabPixmapColor(QCursor::pos()));
    }
    releaseColorGrab();
    emit colorChanged(m_color);
    return true;
}

bool ColorPickerWidget::handleMouseMove(QMouseEvent *e) {
    updateColorNoWheel(grabPixmapColor(e->globalPos()));
    return true;
}

void ColorPickerWidget::updateGrabButton(const bool activated) {
    if (activated) {
        m_colorGrabButton->setText(tr("Press ESC to cancel"));
    } else {
        m_colorGrabButton->setText(tr("Grab Color"));
    }
}
