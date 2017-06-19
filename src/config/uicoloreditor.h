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

#ifndef UICOLORPICKER_H
#define UICOLORPICKER_H

#include "color_wheel.hpp"
#include "src/capture/button.h"
#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;
class Button;
class ClickableLabel;

class UIcolorEditor : public QFrame {
    Q_OBJECT
public:
    explicit UIcolorEditor(QWidget *parent = 0);

private slots:
    void updateUIcolor();
    void updateLocalColor(const QColor);
    void updateButtonIcon();
    void changeLastButton(Button *);

private:
    QColor m_uiColor, m_contrastColor;
    Button *m_buttonMainColor;
    ClickableLabel *m_labelMain;
    Button *m_buttonContrast;
    ClickableLabel *m_labelContrast;
    Button *m_lastButtonPressed;
    color_widgets::ColorWheel *m_colorWheel;

    static const Button::Type m_buttonIconType = Button::Type::circle;

    QHBoxLayout *hLayout;
    QVBoxLayout *vLayout;

    void initColorWheel();
    void initButtons();
};

#endif // UICOLORPICKER_H
