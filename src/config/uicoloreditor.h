// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "QtColorWidgets/color_wheel.hpp"
#include "src/widgets/capture/capturetoolbutton.h"
#include <QGroupBox>

class QVBoxLayout;
class QHBoxLayout;
class CaptureToolButton;
class ClickableLabel;

class UIcolorEditor : public QWidget
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

    static const CaptureTool::Type m_buttonIconType = CaptureTool::TYPE_CIRCLE;

    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;

    void initColorWheel();
    void initButtons();
};
