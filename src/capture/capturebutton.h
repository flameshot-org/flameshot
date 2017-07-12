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
#include <QMap>

class QWidget;
class QPropertyAnimation;

class CaptureButton : public QPushButton {
    Q_OBJECT
    Q_ENUMS(Type)

public:
    enum class Type {
        pencil,
        line,
        arrow,
        selection,
        rectangle,
        circle,
        marker,
        selectionIndicator,
        move,
        undo,
        copy,
        save,
        exit,
        imageUploader,
        last, // used for iteration over the enum
    };

    explicit CaptureButton(const Type, QWidget *parent = 0);
    explicit CaptureButton(const Type, const bool isWhite, QWidget *parent = 0);

    static QIcon getIcon(const Type);
    static QIcon getIcon(const Type, bool isWhite);
    static QString getStyle();
    static QString getStyle(const QColor &);
    static size_t getButtonBaseSize();
    static CaptureButton::Type getTypeByName(const QString);
    static QString getTypeName(const CaptureButton::Type);
    static QString getTypeTooltip(const CaptureButton::Type);

    Type getButtonType() const;

    void updateIconColor(const QColor &);
    void updateIconColor();

    bool iconIsWhite() const;
    static bool iconIsWhite(const QColor &);

    void animatedShow();

protected:
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);

signals:
    void hovered();
    void mouseExited();
    void pressedButton(CaptureButton *);

private:
    CaptureButton(QWidget *parent = 0);
    const Type m_buttonType;
    static const int m_colorValueLimit = 166;
    static const int m_colorSaturationLimit = 110;
    bool m_pressed;

    QPropertyAnimation *emergeAnimation;

    typedef QMap<CaptureButton::Type, const char *> typeData;
    static typeData typeTooltip;
    static typeData typeName;
    static QColor m_mainColor;


    void initButton();
};

#endif // BUTTON_H
