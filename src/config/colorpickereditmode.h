// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include "src/widgets/colorpickerwidget.h"

class ColorPickerEditMode : public ColorPickerWidget
{
    Q_OBJECT
public:
    explicit ColorPickerEditMode(QWidget* parent = nullptr);

signals:
    void colorSelected(int index);

protected:
    void mousePressEvent(QMouseEvent* e) override;
};
