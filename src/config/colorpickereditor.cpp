// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#include "colorpickereditor.h"
#include "colorpickereditmode.h"
#include "src/utils/globalvalues.h"

#include <QApplication>
#include <QColor>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QVector>

ColorPickerEditor::ColorPickerEditor(QWidget* parent)
  : QWidget(parent)
  , m_selectedIndex(1)
{
    m_color = m_config.drawColor();
    m_colorList = m_config.userColors();

    m_gLayout = new QGridLayout(this);

    m_colorpicker = new ColorPickerEditMode(this);
    m_gLayout->addWidget(m_colorpicker, 0, 0);

    m_colorWheel = new color_widgets::ColorWheel(this);
    m_colorWheel->setColor(m_color);
    const int size = GlobalValues::buttonBaseSize() * 3.5;
    m_colorWheel->setMinimumSize(size, size);
    m_gLayout->addWidget(m_colorWheel, 1, 0);

    auto* m_vLocalLayout1 = new QVBoxLayout();
    m_vLocalLayout1->addStretch();

    m_colorEditLabel = new QLabel(tr("Edit Preset:"), this);
    m_vLocalLayout1->addWidget(m_colorEditLabel);

    m_colorEdit = new QLineEdit(this);
    m_colorEdit->setText(m_colorList[m_selectedIndex].name(QColor::HexRgb));
    m_colorEdit->setToolTip(tr("Enter color to update preset"));
    connect(m_colorpicker,
            &ColorPickerEditMode::colorSelected,
            this,
            [this](int index) {
                m_selectedIndex = index;
                m_colorEdit->setText(
                  m_colorList[m_selectedIndex].name(QColor::HexRgb));
            });
    connect(m_colorpicker,
            &ColorPickerEditMode::presetsSwapped,
            this,
            [this](int index) {
                m_selectedIndex = index;
                m_colorList = m_config.userColors();
                m_colorEdit->setText(
                  m_colorList[m_selectedIndex].name(QColor::HexRgb));
            });
    m_vLocalLayout1->addWidget(m_colorEdit);

    m_updatePresetButton = new QPushButton(tr("Update"), this);
    m_updatePresetButton->setToolTip(
      tr("Press button to update the selected preset"));
    connect(m_updatePresetButton,
            &QPushButton::pressed,
            this,
            &ColorPickerEditor::onUpdatePreset);
    m_vLocalLayout1->addWidget(m_updatePresetButton);

    m_deletePresetButton = new QPushButton(tr("Delete"), this);
    m_deletePresetButton->setToolTip(
      tr("Press button to delete the selected preset"));
    connect(m_deletePresetButton,
            &QPushButton::pressed,
            this,
            &ColorPickerEditor::onDeletePreset);
    m_vLocalLayout1->addWidget(m_deletePresetButton);

    m_vLocalLayout1->addStretch();

    m_gLayout->addLayout(m_vLocalLayout1, 0, 1);

    auto* m_vLocalLayout2 = new QVBoxLayout();
    m_vLocalLayout2->addStretch();

    m_addPresetLabel = new QLabel(tr("Add Preset:"), this);
    m_vLocalLayout2->addWidget(m_addPresetLabel);

    m_colorInput = new QLineEdit(this);
    m_colorInput->setText(m_color.name(QColor::HexRgb));
    m_colorInput->setToolTip(
      tr("Enter color manually or select it using the color-wheel"));
    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorSelected,
            this,
            [=](QColor c) {
                m_color = c;
                m_colorInput->setText(m_color.name(QColor::HexRgb));
            });
    m_vLocalLayout2->addWidget(m_colorInput);

    m_addPresetButton = new QPushButton(tr("Add"), this);
    m_addPresetButton->setToolTip(tr("Press button to add preset"));
    connect(m_addPresetButton,
            &QPushButton::pressed,
            this,
            &ColorPickerEditor::onAddPreset);
    m_vLocalLayout2->addWidget(m_addPresetButton);

    m_vLocalLayout2->addStretch();

    m_gLayout->addLayout(m_vLocalLayout2, 1, 1);
}

void ColorPickerEditor::addPreset()
{
    if (m_colorList.contains(m_color)) {
        return;
    }

    const int maxPresetsAllowed = 17;

    if (m_colorList.size() >= maxPresetsAllowed) {
        QMessageBox::critical(
          this,
          tr("Error"),
          tr("Unable to add preset. Maximum limit reached."));
        return;
    }

    m_colorList << m_color;

    m_config.setUserColors(m_colorList);
}

void ColorPickerEditor::deletePreset()
{
    const int minPresetsAllowed = 3;

    if (m_colorList.size() <= minPresetsAllowed) {
        QMessageBox::critical(
          this,
          tr("Error"),
          tr("Unable to remove preset. Minimum limit reached."));
        return;
    }

    m_colorList.remove(m_selectedIndex);

    m_config.setUserColors(m_colorList);
}

void ColorPickerEditor::updatePreset()
{
    QColor c = QColor(m_colorEdit->text());

    if (m_colorList.contains(c)) {
        m_colorEdit->setText(m_colorList[m_selectedIndex].name(QColor::HexRgb));
        return;
    }

    m_colorList[m_selectedIndex] = c;

    m_config.setUserColors(m_colorList);
}

void ColorPickerEditor::onAddPreset()
{
    if (QColor::isValidColor(m_colorInput->text())) {
        m_color = QColor(m_colorInput->text());
        m_colorInput->setText(m_color.name(QColor::HexRgb));
    } else {
        m_colorInput->setText(m_color.name(QColor::HexRgb));
        return;
    }

    addPreset();
    m_colorpicker->updateWidget();
    m_selectedIndex = 1;
    m_colorpicker->updateSelection(m_selectedIndex);
    m_colorEdit->setText(m_colorList[m_selectedIndex].name(QColor::HexRgb));
}

void ColorPickerEditor::onDeletePreset()
{
    deletePreset();
    m_colorpicker->updateWidget();
    m_selectedIndex = 1;
    m_colorpicker->updateSelection(m_selectedIndex);
    m_colorEdit->setText(m_colorList[m_selectedIndex].name(QColor::HexRgb));
}

void ColorPickerEditor::onUpdatePreset()
{
    if (QColor::isValidColor(m_colorEdit->text())) {
        QColor c = QColor(m_colorEdit->text());
        m_colorEdit->setText(c.name(QColor::HexRgb));
    } else {
        m_colorEdit->setText(m_colorList[m_selectedIndex].name(QColor::HexRgb));
        return;
    }

    updatePreset();
    m_colorpicker->updateWidget();
}