// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include "QtColorWidgets/color_wheel.hpp"
#include <QWidget>

class ColorSpinBox;
class ColorPickerWidget;
class QLabel;
class QPushButton;
class QLineEdit;
class QColor;
class QGridLayout;

class ColorPickerEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPickerEditor(QWidget* parent = nullptr);

private slots:
    void onAddPreset();
    void onDeletePreset();

private:
    void addPreset();
    void deletePreset();

    ColorPickerWidget* m_colorpicker;
    color_widgets::ColorWheel* m_colorWheel;

    QLabel* m_colorSpinboxLabel;
    ColorSpinBox* m_colorSpinbox;
    QPushButton* m_deletePresetButton;

    QLineEdit* m_colorInput;
    QLabel* m_addPresetLabel;
    QPushButton* m_addPresetButton;

    QColor m_color;
    int m_selectedIndex;

    QGridLayout* m_gLayout;
};
