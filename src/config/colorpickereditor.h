// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include "QtColorWidgets/color_wheel.hpp"
#include "src/utils/confighandler.h"

#include <QWidget>

class ColorPickerEditMode;
class QCheckBox;
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
    void onUpdatePreset();

private:
    void addPreset();
    void deletePreset();
    void updatePreset();
    void updateDefaultIndex();
    void updateGrabberShown(bool shown);

    const int m_maxPresetsAllowed = 17;
    const int m_minPresetsAllowed = 3;

    ColorPickerEditMode* m_colorpicker;
    color_widgets::ColorWheel* m_colorWheel;

    QCheckBox* m_enableGrabberCheckBox;
    QLabel* m_colorEditLabel;
    QLineEdit* m_colorEdit;
    QPushButton* m_deletePresetButton;
    QPushButton* m_updatePresetButton;

    QLineEdit* m_colorInput;
    QLabel* m_addPresetLabel;
    QPushButton* m_addPresetButton;

    QColor m_color;
    int m_defaultIndex;
    int m_selectedIndex;
    QVector<QColor> m_colorList;
    ConfigHandler m_config;

    QGridLayout* m_gLayout;
};
