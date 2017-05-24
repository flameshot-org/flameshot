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

#include "button.h"
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

Button::Button(const Type t, QWidget *parent) : QPushButton(parent),
    m_buttonType(t), m_pressed(false) {
    initButton();

    if (t == Button::Type::selectionIndicator) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        setIcon(getIcon(t));
    }
}

Button::Button(const Button::Type t, const bool isWhite, QWidget *parent)
    : QPushButton(parent), m_buttonType(t), m_pressed(false) {
    initButton();

    if (t == Button::Type::selectionIndicator) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        setIcon(getIcon(t, isWhite));
    }
}

void Button::initButton() {
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
QIcon Button::getIcon(const Type t, bool isWhite) {
    QString iconColor = "Black";
    if (isWhite) {
        iconColor = "White";
    }
    QString path = ":/img/buttonIcons" + iconColor + "/";

//    if (t == Type::mouseVisibility) {
//        QSettings settings;
//        bool mouseVisible = settings.value("mouseVisible").toBool();
//        if (mouseVisible){
//            path += "mouse.svg";
//        } else {
//            path += "mouse-off.svg";
//        }
//        return QIcon(path);
//    }

    switch (t) {
    case Type::arrow:
        path += "arrow-bottom-left.svg";
        break;
    case Type::circle:
        path += "circle-outline.svg";
        break;
    case Type::colorPicker:
        path += "square-outline.svg";
        break;
    case Type::copy:
        path += "content-copy.svg";
        break;
    case Type::exit:
        path += "close.svg";
        break;
    case Type::imageUploader:
        path += "cloud-upload.svg";
        break;
    case Type::line:
        path += "line.svg";
        break;
    case Type::marker:
        path += "marker.svg";
        break;
    case Type::pencil:
        path += "pencil.svg";
        break;
    case Type::rectangle:
        path += "square-outline.svg";
        break;
    case Type::save:
        path += "content-save.svg";
        break;
    case Type::text:
        path += "format-text.svg";
        break;
    case Type::undo:
        path += "undo-variant.svg";
        break;
    case Type::move:
        path += "cursor-move.svg";
        break;
    default:
        break;
    }
    return QIcon(path);
}

QString Button::getStyle() {
    QSettings settings;
    QColor mainColor = settings.value("uiColor").value<QColor>();
    return getStyle(mainColor);
}

QString Button::getStyle(const QColor &mainColor) {
    QString baseSheet = "Button { border-radius: %3;"
                        "background-color: %1; color: %4 }"
                        "Button:hover { background-color: %2; }"
                        "Button:pressed:!hover { "
                        "background-color: %1; }";
    QColor contrast(mainColor.darker(120));
    if (mainColor.name() < QColor(30,30,30).name()) {
        contrast = contrast.lighter(120);
    }
    bool iconsAreWhite = QSettings().value("whiteIconColor").toBool();
    QString color = "black";
    if (iconsAreWhite) { color = "white"; }

    return baseSheet.arg(mainColor.name()).arg(contrast.name())
            .arg(BUTTON_SIZE/2).arg(color);
}
// get icon returns the icon for the type of button
QIcon Button::getIcon(const Type t) {
    QSettings settings;
    bool isWhite = settings.value("whiteIconColor").toBool();
    return getIcon(t, isWhite);
}

void Button::enterEvent(QEvent *e) {
    Q_EMIT hovered();
    QWidget::enterEvent(e);
}

void Button::leaveEvent(QEvent *e) {
    m_pressed = false;
    Q_EMIT mouseExited();
    QWidget::leaveEvent(e);
}

void Button::mouseReleaseEvent(QMouseEvent *e) {
    CaptureWidget *parent = static_cast<CaptureWidget*>(this->parent());
    parent->mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton && m_pressed) {
//        if (m_buttonType == Type::mouseVisibility) {
//            QSettings settings;
//            bool mouseVisible = settings.value("mouseVisible").toBool();
//            settings.setValue("mouseVisible", !mouseVisible);
//            setIcon(getIcon(Type::mouseVisibility));
//        } else if (m_buttonType == Type::colorPicker) {

//        }
        Q_EMIT pressedButton(this);
    }
    m_pressed = false;
}

void Button::mousePressEvent(QMouseEvent *) {
    m_pressed = true;
}

void Button::animatedShow() {
    show();
    emergeAnimation->start();
}

Button::Type Button::getButtonType() const {
    return m_buttonType;
}
// getButtonBaseSize returns the base size of the buttons
size_t Button::getButtonBaseSize() {
    return BUTTON_SIZE;
}
// getTypeByName receives a name and return the corresponding button type.
// returns Button::Type::last when the corresponding button is not found.
Button::Type Button::getTypeByName(const QString s) {
    Button::Type res = Type::last;
    for (auto i: typeName.toStdMap())
        if (i.second == s)
            res = i.first;
    return res;
}

QString Button::getTypeName(const Button::Type t) {
    return tr(typeName[t]);
}

QString Button::getTypeTooltip(const Button::Type t) {
    return tr(typeTooltip[t]);
}

Button::typeData Button::typeTooltip = {
        {Button::Type::selectionIndicator, QT_TR_NOOP("Shows the dimensions of the selection (X Y)")},
        {Button::Type::mouseVisibility, QT_TR_NOOP("Sets the visibility of the mouse pointer")},
        {Button::Type::exit, QT_TR_NOOP("Leaves the capture screen")},
        {Button::Type::copy, QT_TR_NOOP("Copies the selecion into the clipboard")},
        {Button::Type::save, QT_TR_NOOP("Opens the save image window")},
        {Button::Type::pencil, QT_TR_NOOP("Sets the paint tool to a pencil")},
        {Button::Type::line, QT_TR_NOOP("Sets the paint tool to a line drawer")},
        {Button::Type::arrow, QT_TR_NOOP("Sets the paint tool to an arrow drawer")},
        {Button::Type::rectangle, QT_TR_NOOP("Sets the paint tool to a rectagle drawer")},
        {Button::Type::circle, QT_TR_NOOP("Sets the paint tool to a circle drawer")},
        {Button::Type::marker, QT_TR_NOOP("Sets the paint tool to a marker")},
        {Button::Type::text, QT_TR_NOOP("Sets the paint tool to a text creator")},
        {Button::Type::colorPicker, QT_TR_NOOP("Opens the color picker widget")},
        {Button::Type::undo, QT_TR_NOOP("Undo the last modification")},
        {Button::Type::imageUploader, QT_TR_NOOP("Upload the selection to Imgur")},
        {Button::Type::move, QT_TR_NOOP("Move the selection area")}
};

Button::typeData Button::typeName = {
    {Button::Type::selectionIndicator, QT_TR_NOOP("Selection Size Indicator")},
    {Button::Type::mouseVisibility, QT_TR_NOOP("Mouse Visibility")},
    {Button::Type::exit, QT_TR_NOOP("Exit")},
    {Button::Type::copy, QT_TR_NOOP("Copy")},
    {Button::Type::save, QT_TR_NOOP("Save")},
    {Button::Type::pencil, QT_TR_NOOP("Pencil")},
    {Button::Type::line, QT_TR_NOOP("Line")},
    {Button::Type::arrow, QT_TR_NOOP("Arrow")},
    {Button::Type::rectangle, QT_TR_NOOP("Rectangle")},
    {Button::Type::circle, QT_TR_NOOP("Circle")},
    {Button::Type::marker, QT_TR_NOOP("Marker")},
    {Button::Type::text, QT_TR_NOOP("Text")},
    {Button::Type::colorPicker, QT_TR_NOOP("Color Picker")},
    {Button::Type::undo, QT_TR_NOOP("Undo")},
    {Button::Type::imageUploader, QT_TR_NOOP("Image Uploader")},
    {Button::Type::move, QT_TR_NOOP("Move")}
};
