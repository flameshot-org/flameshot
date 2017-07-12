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
#include <QIcon>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QSettings>
#include <QMouseEvent>

// Button represents a single button of the capture widget, it can enable
// multiple functionality.

namespace {
    const int BUTTON_SIZE = 30;
}

CaptureButton::CaptureButton(const Type t, QWidget *parent) : QPushButton(parent),
    m_buttonType(t), m_pressed(false)
{
    initButton();

    if (t == CaptureButton::Type::selectionIndicator) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        setIcon(getIcon(t));
    }
}

CaptureButton::CaptureButton(const CaptureButton::Type t, const bool isWhite, QWidget *parent)
    : QPushButton(parent), m_buttonType(t), m_pressed(false)
{
    initButton();

    if (t == CaptureButton::Type::selectionIndicator) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        setIcon(getIcon(t, isWhite));
    }
}

void CaptureButton::initButton() {
    setFocusPolicy(Qt::NoFocus);
    resize(BUTTON_SIZE, BUTTON_SIZE);
    setMouseTracking(true);
    setMask(QRegion(QRect(-1,-1,BUTTON_SIZE+2, BUTTON_SIZE+2), QRegion::Ellipse));

    setToolTip(getTypeTooltip(m_buttonType));

    emergeAnimation = new  QPropertyAnimation(this, "size", this);
    emergeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    emergeAnimation->setDuration(80);
    emergeAnimation->setStartValue(QSize(0, 0));
    emergeAnimation->setEndValue(QSize(BUTTON_SIZE, BUTTON_SIZE));
}

// getIcon returns the icon for the type of button, this method lets
// you choose between black or white icons (needed for the config menu)
QIcon CaptureButton::getIcon(const Type t, bool isWhite) {
    QString iconColor = "Black";
    if (isWhite) {
        iconColor = "White";
    }
    QString path = ":/img/buttonIcons" + iconColor + "/";

    switch (t) {
    case Type::arrow:
        path += "arrow-bottom-left.png";
        break;
    case Type::circle:
        path += "circle-outline.png";
        break;
    case Type::copy:
        path += "content-copy.png";
        break;
    case Type::exit:
        path += "close.png";
        break;
    case Type::imageUploader:
        path += "cloud-upload.png";
        break;
    case Type::line:
        path += "line.png";
        break;
    case Type::marker:
        path += "marker.png";
        break;
    case Type::pencil:
        path += "pencil.png";
        break;
    case Type::selection:
        path += "square-outline.png";
        break;
    case Type::save:
        path += "content-save.png";
        break;
    case Type::undo:
        path += "undo-variant.png";
        break;
    case Type::move:
        path += "cursor-move.png";
        break;
    case Type::rectangle:
        path += "square.png";
        break;
    default:
        break;
    }
    return QIcon(path);
}

QString CaptureButton::getStyle() {
    QSettings settings;
    m_mainColor = settings.value("uiColor").value<QColor>();
    return getStyle(m_mainColor);
}

QString CaptureButton::getStyle(const QColor &mainColor) {
    m_mainColor = mainColor;
    QString baseSheet = "Button { border-radius: %3;"
                        "background-color: %1; color: %4 }"
                        "Button:hover { background-color: %2; }"
                        "Button:pressed:!hover { "
                        "background-color: %1; }";

    // define color when mouse is hovering
    QColor contrast(mainColor.darker(120));
    if (mainColor.value() < m_colorValueLimit ||
            mainColor.saturation() > m_colorSaturationLimit) {
        contrast = mainColor.lighter(140);
    }

    // foreground color
    QString color = iconIsWhite(mainColor) ? "white" : "black";

    return baseSheet.arg(mainColor.name()).arg(contrast.name())
            .arg(BUTTON_SIZE/2).arg(color);
}
// get icon returns the icon for the type of button
QIcon CaptureButton::getIcon(const Type t) {
    return getIcon(t, iconIsWhite(m_mainColor));
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
}

void CaptureButton::animatedShow() {
    show();
    emergeAnimation->start();
}

CaptureButton::Type CaptureButton::getButtonType() const {
    return m_buttonType;
}

void CaptureButton::updateIconColor(const QColor &c) {
    setIcon(getIcon(m_buttonType, iconIsWhite(c)));
}

void CaptureButton::updateIconColor() {
    setIcon(getIcon(m_buttonType, iconIsWhite()));
}
// iconIsWhite returns true if the passed color would contain a white icon
// if applied to a button, and false otherwise
bool CaptureButton::iconIsWhite(const QColor &c) {
    bool isWhite = false;
    if (c.value() < m_colorValueLimit ||
            c.saturation() > m_colorSaturationLimit) {
        isWhite = true;
    }
    return isWhite;
}

bool CaptureButton::iconIsWhite() const {
    return iconIsWhite(m_mainColor);
}
// getButtonBaseSize returns the base size of the buttons
size_t CaptureButton::getButtonBaseSize() {
    return BUTTON_SIZE;
}
// getTypeByName receives a name and return the corresponding button type.
// returns Button::Type::last when the corresponding button is not found.
CaptureButton::Type CaptureButton::getTypeByName(const QString s) {
    CaptureButton::Type res = Type::last;
    for (auto i: typeName.toStdMap())
        if (tr(i.second) == s)
            res = i.first;
    return res;
}

QString CaptureButton::getTypeName(const CaptureButton::Type t) {
    return tr(typeName[t]);
}

QString CaptureButton::getTypeTooltip(const CaptureButton::Type t) {
    return tr(typeTooltip[t]);
}

CaptureButton::typeData CaptureButton::typeTooltip = {
        {CaptureButton::Type::selectionIndicator, QT_TR_NOOP("Shows the dimensions of the selection (X Y)")},
        {CaptureButton::Type::exit, QT_TR_NOOP("Leaves the capture screen")},
        {CaptureButton::Type::copy, QT_TR_NOOP("Copies the selecion into the clipboard")},
        {CaptureButton::Type::save, QT_TR_NOOP("Opens the save image window")},
        {CaptureButton::Type::pencil, QT_TR_NOOP("Sets the Pencil as the paint tool")},
        {CaptureButton::Type::line, QT_TR_NOOP("Sets the Line as the paint tool")},
        {CaptureButton::Type::arrow, QT_TR_NOOP("Sets the Arrow as the paint tool")},
        {CaptureButton::Type::rectangle, QT_TR_NOOP("Sets the Rectangle as the paint tool")},
        {CaptureButton::Type::circle, QT_TR_NOOP("Sets the Circle as the paint tool")},
        {CaptureButton::Type::marker, QT_TR_NOOP("Sets the Marker as the paint tool")},
        {CaptureButton::Type::undo, QT_TR_NOOP("Undo the last modification")},
        {CaptureButton::Type::imageUploader, QT_TR_NOOP("Uploads the selection to Imgur")},
        {CaptureButton::Type::selection, QT_TR_NOOP("Sets the Selection as the paint tool")},
        {CaptureButton::Type::move, QT_TR_NOOP("Move the selection area")}
};

CaptureButton::typeData CaptureButton::typeName = {
    {CaptureButton::Type::selectionIndicator, QT_TR_NOOP("Selection Size Indicator")},
    {CaptureButton::Type::exit, QT_TR_NOOP("Exit")},
    {CaptureButton::Type::copy, QT_TR_NOOP("Copy")},
    {CaptureButton::Type::save, QT_TR_NOOP("Save")},
    {CaptureButton::Type::pencil, QT_TR_NOOP("Pencil")},
    {CaptureButton::Type::line, QT_TR_NOOP("Line")},
    {CaptureButton::Type::arrow, QT_TR_NOOP("Arrow")},
    {CaptureButton::Type::rectangle, QT_TR_NOOP("Rectangle")},
    {CaptureButton::Type::circle, QT_TR_NOOP("Circle")},
    {CaptureButton::Type::marker, QT_TR_NOOP("Marker")},
    {CaptureButton::Type::undo, QT_TR_NOOP("Undo")},
    {CaptureButton::Type::imageUploader, QT_TR_NOOP("Image Uploader")},
    {CaptureButton::Type::selection, QT_TR_NOOP("Rectangular Selection")},
    {CaptureButton::Type::move, QT_TR_NOOP("Move")}
};

QColor CaptureButton::m_mainColor = QSettings().value("uiColor").value<QColor>();
