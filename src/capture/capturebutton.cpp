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

#include "capturebutton.h"
#include "src/capture/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/capture/tools/capturetool.h"
#include "src/capture/tools/toolfactory.h"
#include <QIcon>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QMouseEvent>
// Button represents a single button of the capture widget, it can enable
// multiple functionality.

namespace {

const int BUTTON_SIZE = 30;

inline qreal getColorLuma(const QColor &c) {
    return 0.30 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF();
}

QColor getContrastColor(const QColor &c) {
    bool isWhite = CaptureButton::iconIsWhiteByColor(c);
    int change = isWhite ? 30 : -45;

    return QColor(qBound(0, c.red() + change, 255),
                  qBound(0, c.green() + change, 255),
                  qBound(0, c.blue() + change, 255));
}

} // unnamed namespace

CaptureButton::CaptureButton(const ButtonType t, QWidget *parent) : QPushButton(parent),
    m_buttonType(t), m_pressed(false)
{
    initButton();
    if (t == TYPE_SELECTIONINDICATOR) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        setIcon(icon());
    }
}

void CaptureButton::initButton() {
    setMouseTracking(true);
    m_tool = ToolFactory().CreateTool(m_buttonType, this);
    connect(this, &CaptureButton::pressed, m_tool, &CaptureTool::onPressed);

    setFocusPolicy(Qt::NoFocus);
    resize(BUTTON_SIZE, BUTTON_SIZE);
    setMask(QRegion(QRect(-1,-1,BUTTON_SIZE+2, BUTTON_SIZE+2), QRegion::Ellipse));

    setToolTip(m_tool->description());

    emergeAnimation = new  QPropertyAnimation(this, "size", this);
    emergeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    emergeAnimation->setDuration(80);
    emergeAnimation->setStartValue(QSize(0, 0));
    emergeAnimation->setEndValue(QSize(BUTTON_SIZE, BUTTON_SIZE));
}

QVector<CaptureButton::ButtonType> CaptureButton::getIterableButtonTypes() {
    return iterableButtonTypes;
}

QString CaptureButton::globalStyleSheet() {
    QColor mainColor = ConfigHandler().uiMainColorValue();
    QString baseSheet = "CaptureButton { border-radius: %3;"
                        "background-color: %1; color: %4 }"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = getContrastColor(m_mainColor);

    // foreground color
    QString color = iconIsWhiteByColor(mainColor) ? "white" : "black";

    return baseSheet.arg(mainColor.name()).arg(contrast.name())
            .arg(BUTTON_SIZE/2).arg(color);
}

QString CaptureButton::styleSheet() const {
    QString baseSheet = "CaptureButton { border-radius: %3;"
                        "background-color: %1; color: %4 }"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = getContrastColor(m_mainColor);
    // foreground color
    QString color = iconIsWhiteByColor(m_mainColor) ? "white" : "black";

    return baseSheet.arg(m_mainColor.name()).arg(contrast.name())
            .arg(BUTTON_SIZE/2).arg(color);
}

// get icon returns the icon for the type of button
QIcon CaptureButton::icon() const {
    QString color(iconIsWhiteByColor(m_mainColor) ? "White" : "Black");
    QString iconPath = QString(":/img/buttonIcons%1/%2")
            .arg(color).arg(m_tool->iconName());
    return QIcon(iconPath);
}

void CaptureButton::enterEvent(QEvent *e) {
    Q_EMIT hovered();
    QWidget::enterEvent(e);
}

void CaptureButton::leaveEvent(QEvent *e) {
    m_pressed = false;
    Q_EMIT mouseExited();
    QWidget::leaveEvent(e);
}

void CaptureButton::mouseReleaseEvent(QMouseEvent *e) {
    CaptureWidget *parent = static_cast<CaptureWidget*>(this->parent());
    parent->mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton && m_pressed) {
        Q_EMIT pressedButton(this);
    }
    m_pressed = false;
}

void CaptureButton::mousePressEvent(QMouseEvent *) {
    m_pressed = true;
    Q_EMIT pressed();
}


void CaptureButton::animatedShow() {
    if(!isVisible()) {
        setMouseTracking(false);
        show();
        emergeAnimation->start();
        connect(emergeAnimation, &QPropertyAnimation::finished, this, [this](){
            setMouseTracking(true);
        });
    }
}

CaptureButton::ButtonType CaptureButton::buttonType() const {
    return m_buttonType;
}

CaptureTool *CaptureButton::tool() const {
    return m_tool;
}

void CaptureButton::setColor(const QColor &c) {
    m_mainColor = c;
    setStyleSheet(styleSheet());
    setIcon(icon());
}

// getButtonBaseSize returns the base size of the buttons
size_t CaptureButton::buttonBaseSize() {
    return BUTTON_SIZE;
}

bool CaptureButton::iconIsWhiteByColor(const QColor &c) {
    bool isWhite = false;
    if (getColorLuma(c) <= 0.60) {
        isWhite = true;
    }
    return isWhite;
}

QColor CaptureButton::m_mainColor = ConfigHandler().uiMainColorValue();

QVector<CaptureButton::ButtonType> CaptureButton::iterableButtonTypes = {
    CaptureButton::TYPE_PENCIL,
    CaptureButton::TYPE_LINE,
    CaptureButton::TYPE_ARROW,
    CaptureButton::TYPE_SELECTION,
    CaptureButton::TYPE_RECTANGLE,
    CaptureButton::TYPE_CIRCLE,
    CaptureButton::TYPE_MARKER,
    CaptureButton::TYPE_SELECTIONINDICATOR,
    CaptureButton::TYPE_MOVESELECTION,
    CaptureButton::TYPE_UNDO,
    CaptureButton::TYPE_COPY,
    CaptureButton::TYPE_SAVE,
    CaptureButton::TYPE_EXIT,
    CaptureButton::TYPE_IMAGEUPLOADER,
};
