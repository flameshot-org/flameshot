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
#include <QIcon>
#include <QPropertyAnimation>
#include <QToolTip>
#include <QSettings>

// Button represents a single button of the capture widget, it can enable
// multiple functionality.

namespace {
    const int BUTTON_SIZE = 30;
}

Button::Button(Type t, QWidget *parent) : QPushButton(parent) {
    setFocusPolicy(Qt::NoFocus);
    m_buttonType = t;
    resize(BUTTON_SIZE, BUTTON_SIZE);
    setMouseTracking(true);
    setMask(QRegion(QRect(-1,-1,BUTTON_SIZE+2, BUTTON_SIZE+2), QRegion::Ellipse));

    if (t == Button::Type::selectionIndicator) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        setIcon(getIcon(t));
    }
    setToolTip(typeTooltip[t]);

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

    if (t == Type::mouseVisibility) {
        QSettings settings;
        bool mouseVisible = settings.value("mouseVisible").toBool();
        if (mouseVisible){
            path += "mouse.svg";
        } else {
            path += "mouse-off.svg";
        }
        return QIcon(path);
    }

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
// get icon returns the icon for the type of button
QIcon Button::getIcon(const Type t) {
    // assign the isWhite based on the settings
    bool isWhite = true;
    return getIcon(t, isWhite);
}

void Button::enterEvent(QEvent *e) {
    Q_EMIT hovered();
    QWidget::enterEvent(e);
}

void Button::leaveEvent(QEvent *e) {
    Q_EMIT mouseExited();
    QWidget::leaveEvent(e);
}

void Button::mouseReleaseEvent(QMouseEvent *) {
    if (m_buttonType == Type::mouseVisibility) {
        QSettings settings;
        bool mouseVisible = settings.value("mouseVisible").toBool();
        settings.setValue("mouseVisible", !mouseVisible);
        setIcon(getIcon(Type::mouseVisibility));
    } else if (m_buttonType == Type::colorPicker) {

    }
    Q_EMIT typeEmited(m_buttonType);
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
Button::Type Button::getTypeByName(QString s) {
    Button::Type res = Type::last;
    for (auto it = typeName.begin(); it != typeName.end(); ++it )
        if (it->second == s)
            res = it->first;
    return res;
}

QString Button::getTypeName(Button::Type t) {
    return typeName[t];
}

QString Button::getTypeTooltip(Button::Type t) {
    return typeTooltip[t];
}

Button::typeData Button::typeTooltip = {
        {Button::Type::selectionIndicator, "Shows the dimensions of the selection (X Y)"},
        {Button::Type::mouseVisibility, "Sets the visibility of the mouse pointer"},
        {Button::Type::exit, "Leaves the capture screen"},
        {Button::Type::copy, "Copies the selecion into the clipboard"},
        {Button::Type::save, "Opens the save image window"},
        {Button::Type::pencil, "Sets the paint tool to a pencil"},
        {Button::Type::line, "Sets the paint tool to a line drawer"},
        {Button::Type::arrow, "Sets the paint tool to an arrow drawer"},
        {Button::Type::rectangle, "Sets the paint tool to a rectagle drawer"},
        {Button::Type::circle, "Sets the paint tool to a circle drawer"},
        {Button::Type::marker, "Sets the paint tool to a marker"},
        {Button::Type::text, "Sets the paint tool to a text creator"},
        {Button::Type::colorPicker, "Opens the color picker widget"},
        {Button::Type::undo, "Undo the last modification"},
        {Button::Type::imageUploader, "Upload the selection to Imgur"},
        {Button::Type::move, "Move the selection area"}
    };

Button::typeData Button::typeName = {
    {Button::Type::selectionIndicator, "Selection Size Indicator"},
    {Button::Type::mouseVisibility, "Mouse Visibility"},
    {Button::Type::exit, "Exit"},
    {Button::Type::copy, "Copy"},
    {Button::Type::save, "Save"},
    {Button::Type::pencil, "Pencil"},
    {Button::Type::line, "Line"},
    {Button::Type::arrow, "Arrow"},
    {Button::Type::rectangle, "Rectangle"},
    {Button::Type::circle, "Circle"},
    {Button::Type::marker, "Marker"},
    {Button::Type::text, "Text"},
    {Button::Type::colorPicker, "Color Picker"},
    {Button::Type::undo, "Undo"},
    {Button::Type::imageUploader, "Image Uploader"},
    {Button::Type::move, "Move"}
};
