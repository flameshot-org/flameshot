// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#include "colorpickereditor.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/widgets/colorpickerwidget.h"
#include "src/widgets/colorspinbox.h"

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
    ConfigHandler config;
    m_color = config.drawColor();

    m_gLayout = new QGridLayout(this);

    m_colorpicker = new ColorPickerWidget(this);
    m_gLayout->addWidget(m_colorpicker, 0, 0);

    m_colorWheel = new color_widgets::ColorWheel(this);
    m_colorWheel->setColor(m_color);
    const int size = GlobalValues::buttonBaseSize() * 3.5;
    m_colorWheel->setMinimumSize(size, size);
    m_gLayout->addWidget(m_colorWheel, 1, 0);

    auto* m_vLocalLayout1 = new QVBoxLayout();
    m_vLocalLayout1->addStretch();

    m_colorSpinboxLabel = new QLabel(tr("Select Preset:"), this);
    m_vLocalLayout1->addWidget(m_colorSpinboxLabel);

    m_colorSpinbox = new ColorSpinBox(this);
    connect(m_colorSpinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_colorpicker,
            [=](int val) {
                m_selectedIndex = val;
                m_colorpicker->updateSelection(val);
            });
    m_colorSpinbox->setToolTip(tr("Select preset using the spinbox"));
    m_vLocalLayout1->addWidget(m_colorSpinbox);

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
    ConfigHandler config;
    QVector<QColor> colors = config.userColors();

    if (colors.contains(m_color)) {
        return;
    }

    colors << m_color;

    const int maxPresetsAllowed = 17;

    if (colors.size() > maxPresetsAllowed) {
        QMessageBox::critical(
          this,
          tr("Error"),
          tr("Unable to add preset. Maximum limit reached."));
        return;
    }

    config.setUserColors(colors);
}

void ColorPickerEditor::deletePreset()
{
    ConfigHandler config;
    QVector<QColor> colors = config.userColors();

    colors.remove(m_selectedIndex);

    const int minPresetsAllowed = 3;

    if (colors.size() < minPresetsAllowed) {
        QMessageBox::critical(
          this,
          tr("Error"),
          tr("Unable to remove preset. Minimum limit reached."));
        return;
    }

    config.setUserColors(colors);
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
    m_colorSpinbox->setValue(1);
    m_colorpicker->updateWidget();
    m_colorSpinbox->updateWidget();
}

void ColorPickerEditor::onDeletePreset()
{
    deletePreset();
    m_colorSpinbox->setValue(1);
    m_colorpicker->updateWidget();
    m_colorSpinbox->updateWidget();
}