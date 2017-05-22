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

#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <map>

class QWidget;
class QPropertyAnimation;

class Button : public QPushButton {
    Q_OBJECT

public:
    enum class Type {
        selectionIndicator,
        exit,
        copy,
        save,
        pencil,
        line,
        arrow,
        rectangle,
        circle,
        marker,
        text,
        undo,
        imageUploader,
        move,
        last, // used for iteration over the enum
        mouseVisibility,
        colorPicker
    };

    explicit Button(const Type, QWidget *parent = 0);
    explicit Button(const Type, const bool isWhite, QWidget *parent = 0);

    static QIcon getIcon(const Type);
    static QIcon getIcon(const Type, bool isWhite);
    static QString getStyle();
    static QString getStyle(const QColor &);
    static size_t getButtonBaseSize();
    static Button::Type getTypeByName(const QString);
    static QString getTypeName(const Button::Type);
    static QString getTypeTooltip(const Button::Type);

    Type getButtonType() const;

    void animatedShow();

protected:
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);

signals:
    void hovered();
    void mouseExited();
    void typeEmited(Type);

private:
    Button(QWidget *parent = 0);
    Type m_buttonType;
    bool m_pressed;

    QPropertyAnimation *emergeAnimation;

    typedef std::map<Button::Type, const QString> typeData;
    static typeData typeTooltip;
    static typeData typeName;

    void initButton();
};

#endif // BUTTON_H
