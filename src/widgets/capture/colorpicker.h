// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/widgets/colorpickerwidget.h"

class ColorPicker : public ColorPickerWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget* parent = nullptr);

signals:
    void colorSelected(QColor c);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void mouseMoveEvent(QMouseEvent* e) override;
};
