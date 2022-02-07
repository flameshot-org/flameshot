// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include "QtColorWidgets/color_wheel.hpp"
#include <QWidget>

class ColorPickerEditMode;
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

    ColorPickerEditMode* m_colorpicker;
    color_widgets::ColorWheel* m_colorWheel;

    QLabel* m_colorEditLabel;
    QPushButton* m_deletePresetButton;

    QLineEdit* m_colorInput;
    QLabel* m_addPresetLabel;
    QPushButton* m_addPresetButton;

    QColor m_color;
    int m_selectedIndex;

    QGridLayout* m_gLayout;
};
