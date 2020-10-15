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

#include "color_wheel.hpp"
#include "src/widgets/capture/capturetoolbutton.h"
#include <QGroupBox>

class QVBoxLayout;
class QHBoxLayout;
class CaptureToolButton;
class ClickableLabel;

class UIcolorEditor : public QGroupBox
{
    Q_OBJECT
public:
    explicit UIcolorEditor(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void updateUIcolor();
    void updateLocalColor(const QColor);
    void changeLastButton(CaptureToolButton*);

private:
    QColor m_uiColor, m_contrastColor;
    CaptureToolButton* m_buttonMainColor;
    ClickableLabel* m_labelMain;
    CaptureToolButton* m_buttonContrast;
    ClickableLabel* m_labelContrast;
    CaptureToolButton* m_lastButtonPressed;
    color_widgets::ColorWheel* m_colorWheel;

    static const CaptureToolButton::ButtonType m_buttonIconType =
      CaptureToolButton::TYPE_CIRCLE;

    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;

    void initColorWheel();
    void initButtons();
};
