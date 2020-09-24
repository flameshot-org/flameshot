// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#pragma once

#include "capturebutton.h"
#include <QMap>
#include <QVector>

class QWidget;
class QPropertyAnimation;
class CaptureTool;

class CaptureToolButton : public CaptureButton
{
    Q_OBJECT

public:
    // Don't forget to add the new types to CaptureButton::iterableButtonTypes
    // in the .cpp and the order value in the private array buttonTypeOrder
    enum ButtonType
    {
        TYPE_PENCIL = 0,
        TYPE_DRAWER = 1,
        TYPE_ARROW = 2,
        TYPE_SELECTION = 3,
        TYPE_RECTANGLE = 4,
        TYPE_CIRCLE = 5,
        TYPE_MARKER = 6,
        TYPE_SELECTIONINDICATOR = 7,
        TYPE_MOVESELECTION = 8,
        TYPE_UNDO = 9,
        TYPE_COPY = 10,
        TYPE_SAVE = 11,
        TYPE_EXIT = 12,
        TYPE_IMAGEUPLOADER = 13,
        TYPE_OPEN_APP = 14,
        TYPE_PIXELATE = 15,
        TYPE_REDO = 16,
        TYPE_PIN = 17,
        TYPE_TEXT = 18,
        TYPE_CIRCLECOUNT = 19,

    };
    Q_ENUM(ButtonType)

    explicit CaptureToolButton(const ButtonType, QWidget* parent = nullptr);

    static QVector<CaptureToolButton::ButtonType> getIterableButtonTypes();
    static int getPriorityByButton(CaptureToolButton::ButtonType);

    QString name() const;
    QString description() const;
    QIcon icon() const;
    CaptureTool* tool() const;

    void setColor(const QColor& c);
    void animatedShow();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    static QVector<ButtonType> iterableButtonTypes;

    CaptureTool* m_tool;

signals:
    void pressedButton(CaptureToolButton*);

private:
    CaptureToolButton(QWidget* parent = nullptr);
    ButtonType m_buttonType;

    QPropertyAnimation* m_emergeAnimation;

    static QColor m_mainColor;

    void initButton();
    void updateIcon();
};
