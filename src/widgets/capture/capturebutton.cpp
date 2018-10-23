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

#include "capturebutton.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/tools/capturetool.h"
#include "src/tools/toolfactory.h"
#include "src/utils/globalvalues.h"
#include "src/utils/colorutils.h"
#include <QIcon>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

// Button represents a single button of the capture widget, it can enable
// multiple functionality.

CaptureButton::CaptureButton(const ButtonType t, QWidget *parent) : QPushButton(parent),
    m_buttonType(t)
{
    initButton();
    if (t == TYPE_SELECTIONINDICATOR) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        updateIcon();
    }
    setCursor(Qt::ArrowCursor);
}

void CaptureButton::initButton() {
    m_tool = ToolFactory().CreateTool(m_buttonType, this);

    setFocusPolicy(Qt::NoFocus);
    resize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize());
    setMask(QRegion(QRect(-1,-1, GlobalValues::buttonBaseSize()+2,
                          GlobalValues::buttonBaseSize()+2),
                    QRegion::Ellipse));

    setToolTip(m_tool->description());

    m_emergeAnimation = new  QPropertyAnimation(this, "size", this);
    m_emergeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_emergeAnimation->setDuration(80);
    m_emergeAnimation->setStartValue(QSize(0, 0));
    m_emergeAnimation->setEndValue(
                QSize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize()));

    auto dsEffect = new QGraphicsDropShadowEffect(this);
    dsEffect->setBlurRadius(5);
    dsEffect->setOffset(0);
    dsEffect->setColor(QColor(Qt::black));

    setGraphicsEffect(dsEffect);

}

void CaptureButton::updateIcon() {
    setIcon(icon());
    setIconSize(size()*0.6);
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
    QColor contrast = ColorUtils::contrastColor(m_mainColor);

    // foreground color
    QString color = ColorUtils::colorIsDark(mainColor) ? "white" : "black";

    return baseSheet.arg(mainColor.name()).arg(contrast.name())
            .arg(GlobalValues::buttonBaseSize()/2).arg(color);
}

QString CaptureButton::styleSheet() const {
    QString baseSheet = "CaptureButton { border-radius: %3;"
                        "background-color: %1; color: %4 }"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = ColorUtils::contrastColor(m_mainColor);
    // foreground color
    QString color = ColorUtils::colorIsDark(m_mainColor) ? "white" : "black";

    return baseSheet.arg(m_mainColor.name()).arg(contrast.name())
            .arg(GlobalValues::buttonBaseSize()/2).arg(color);
}

// get icon returns the icon for the type of button
QIcon CaptureButton::icon() const {
    return m_tool->icon(m_mainColor, true);
}

void CaptureButton::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        emit pressedButton(this);
        emit pressed();
    }
}

void CaptureButton::animatedShow() {
    if(!isVisible()) {
        show();
        m_emergeAnimation->start();
        connect(m_emergeAnimation, &QPropertyAnimation::finished, this, [](){
        });
    }
}

CaptureTool *CaptureButton::tool() const {
    return m_tool;
}

void CaptureButton::setColor(const QColor &c) {
    m_mainColor = c;
    setStyleSheet(styleSheet());
    updateIcon();
}

QColor CaptureButton::m_mainColor = ConfigHandler().uiMainColorValue();

static std::map<CaptureButton::ButtonType, int> buttonTypeOrder {
    { CaptureButton::TYPE_PENCIL,             0 },
    { CaptureButton::TYPE_DRAWER,             1 },
    { CaptureButton::TYPE_ARROW,              2 },
    { CaptureButton::TYPE_SELECTION,          3 },
    { CaptureButton::TYPE_RECTANGLE,          4 },
    { CaptureButton::TYPE_CIRCLE,             5 },
    { CaptureButton::TYPE_MARKER,             6 },
    { CaptureButton::TYPE_TEXT,               7 },
    { CaptureButton::TYPE_BLUR,               8 },
    { CaptureButton::TYPE_SELECTIONINDICATOR, 9 },
    { CaptureButton::TYPE_MOVESELECTION,     10 },
    { CaptureButton::TYPE_UNDO,              11 },
    { CaptureButton::TYPE_REDO,              12 },
    { CaptureButton::TYPE_COPY,              13 },
    { CaptureButton::TYPE_SAVE,              14 },
    { CaptureButton::TYPE_EXIT,              15 },
    { CaptureButton::TYPE_IMAGEUPLOADER,     16 },
    { CaptureButton::TYPE_OPEN_APP,          17 },
    { CaptureButton::TYPE_PIN,               18 },
};

int CaptureButton::getPriorityByButton(CaptureButton::ButtonType b) {
    auto it = buttonTypeOrder.find(b);
    return it == buttonTypeOrder.cend() ? (int)buttonTypeOrder.size() : it->second;
}

QVector<CaptureButton::ButtonType> CaptureButton::iterableButtonTypes = {
    CaptureButton::TYPE_PENCIL,
    CaptureButton::TYPE_DRAWER,
    CaptureButton::TYPE_ARROW,
    CaptureButton::TYPE_SELECTION,
    CaptureButton::TYPE_RECTANGLE,
    CaptureButton::TYPE_CIRCLE,
    CaptureButton::TYPE_MARKER,
    CaptureButton::TYPE_TEXT,
    CaptureButton::TYPE_BLUR,
    CaptureButton::TYPE_SELECTIONINDICATOR,
    CaptureButton::TYPE_MOVESELECTION,
    CaptureButton::TYPE_UNDO,
    CaptureButton::TYPE_REDO,
    CaptureButton::TYPE_COPY,
    CaptureButton::TYPE_SAVE,
    CaptureButton::TYPE_EXIT,
    CaptureButton::TYPE_IMAGEUPLOADER,
    CaptureButton::TYPE_OPEN_APP,
    CaptureButton::TYPE_PIN,
};
