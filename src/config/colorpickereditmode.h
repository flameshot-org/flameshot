// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include "src/utils/confighandler.h"
#include "src/widgets/colorpickerwidget.h"

class ColorPickerEditMode : public ColorPickerWidget
{
    Q_OBJECT
public:
    explicit ColorPickerEditMode(QWidget* parent = nullptr);

signals:
    void colorSelected(int index);
    void presetsSwapped(int index);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    bool m_isPressing = false;
    bool m_isDragging = false;
    QPoint m_mouseMovePos;
    QPoint m_mousePressPos;
    QPoint m_draggedPresetInitialPos;
    ConfigHandler m_config;
};
