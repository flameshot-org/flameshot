// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "QtColorWidgets/color_wheel.hpp"
#include <QWidget>

class SpinBox;
class ColorPickerWidget;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QGridLayout;

class ColorPickerEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPickerEditor(QWidget* parent = nullptr);

private:
    QLabel* m_spinboxLabel;
    SpinBox* m_spinbox;
    ColorPickerWidget* m_colorpicker;
    color_widgets::ColorWheel* m_colorWheel;

    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;
    QGridLayout* m_gLayout;
};
