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
#include "src/capture/widgets/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/capture/tools/capturetool.h"
#include "src/capture/tools/toolfactory.h"
#include <QIcon>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

// Button represents a single button of the capture widget, it can enable
// multiple functionality.

namespace {

qreal getColorLuma(const QColor &c) {
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
    connect(this, &CaptureButton::pressed, m_tool, &CaptureTool::onPressed);

    setFocusPolicy(Qt::NoFocus);
    resize(buttonBaseSize(), buttonBaseSize());
    setMask(QRegion(QRect(-1,-1, buttonBaseSize()+2, buttonBaseSize()+2), QRegion::Ellipse));

    setToolTip(m_tool->description());

    m_emergeAnimation = new  QPropertyAnimation(this, "size", this);
    m_emergeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_emergeAnimation->setDuration(80);
    m_emergeAnimation->setStartValue(QSize(0, 0));
    m_emergeAnimation->setEndValue(QSize(buttonBaseSize(), buttonBaseSize()));

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
    QColor contrast = getContrastColor(m_mainColor);

    // foreground color
    QString color = iconIsWhiteByColor(mainColor) ? "white" : "black";

    return baseSheet.arg(mainColor.name()).arg(contrast.name())
            .arg(buttonBaseSize()/2).arg(color);
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
            .arg(buttonBaseSize()/2).arg(color);
}

// get icon returns the icon for the type of button
QIcon CaptureButton::icon() const {
    QString color(iconIsWhiteByColor(m_mainColor) ? "White" : "Black");
    QString iconPath = QStringLiteral(":/img/buttonIcons%1/%2")
            .arg(color).arg(m_tool->iconName());
    return QIcon(iconPath);
}

void CaptureButton::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        Q_EMIT pressedButton(this);
        Q_EMIT pressed();
    }
}

void CaptureButton::animatedShow() {
    if(!isVisible()) {
        show();
        m_emergeAnimation->start();
        connect(m_emergeAnimation, &QPropertyAnimation::finished, this, [this](){
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
    updateIcon();
}

// getButtonBaseSize returns the base size of the buttons
int CaptureButton::buttonBaseSize() {
    return QApplication::fontMetrics().lineSpacing() * 2.2;
}

bool CaptureButton::iconIsWhiteByColor(const QColor &c) {
    bool isWhite = false;
    if (getColorLuma(c) <= 0.60) {
        isWhite = true;
    }
    return isWhite;
}

QColor CaptureButton::m_mainColor = ConfigHandler().uiMainColorValue();

static std::map<CaptureButton::ButtonType, int> buttonTypeOrder {
    { CaptureButton::TYPE_PENCIL,             0 },
    { CaptureButton::TYPE_LINE,               1 },
    { CaptureButton::TYPE_ARROW,              2 },
    { CaptureButton::TYPE_SELECTION,          3 },
    { CaptureButton::TYPE_RECTANGLE,          4 },
    { CaptureButton::TYPE_CIRCLE,             5 },
    { CaptureButton::TYPE_MARKER,             6 },
    { CaptureButton::TYPE_SELECTIONINDICATOR, 8 },
    { CaptureButton::TYPE_MOVESELECTION,      9 },
    { CaptureButton::TYPE_UNDO,              10 },
    { CaptureButton::TYPE_COPY,              11 },
    { CaptureButton::TYPE_SAVE,              12 },
    { CaptureButton::TYPE_EXIT,              13 },
    { CaptureButton::TYPE_IMAGEUPLOADER,     14 },
    { CaptureButton::TYPE_OPEN_APP,          15 },
    { CaptureButton::TYPE_BLUR,              7  },
};

int CaptureButton::getPriorityByButton(CaptureButton::ButtonType b) {
    auto it = buttonTypeOrder.find(b);
    return it == buttonTypeOrder.cend() ? (int)buttonTypeOrder.size() : it->second;
}

QVector<CaptureButton::ButtonType> CaptureButton::iterableButtonTypes = {
    CaptureButton::TYPE_PENCIL,
    CaptureButton::TYPE_LINE,
    CaptureButton::TYPE_ARROW,
    CaptureButton::TYPE_SELECTION,
    CaptureButton::TYPE_RECTANGLE,
    CaptureButton::TYPE_CIRCLE,
    CaptureButton::TYPE_MARKER,
    CaptureButton::TYPE_BLUR,
    CaptureButton::TYPE_SELECTIONINDICATOR,
    CaptureButton::TYPE_MOVESELECTION,
    CaptureButton::TYPE_UNDO,
    CaptureButton::TYPE_COPY,
    CaptureButton::TYPE_SAVE,
    CaptureButton::TYPE_EXIT,
    CaptureButton::TYPE_IMAGEUPLOADER,
    CaptureButton::TYPE_OPEN_APP,
};
