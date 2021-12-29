// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorpickereditor.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/widgets/colorpickerwidget.h"
#include "src/widgets/spinbox.h"

#include <QApplication>
#include <QColor>
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

    m_hLayout = new QHBoxLayout(this);

    m_colorWheel = new color_widgets::ColorWheel(this);
    m_colorWheel->setColor(m_color);

    m_colorpicker = new ColorPickerWidget(this);

    m_hLayout->addWidget(m_colorWheel);
    m_hLayout->addWidget(m_colorpicker);

    m_vLayout = new QVBoxLayout();
    m_vLayout->setAlignment(Qt::AlignVCenter);

    m_spinboxLabel = new QLabel(tr("Select Preset:"), this);
    m_spinbox = new SpinBox(this);

    connect(m_spinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_colorpicker,
            &ColorPickerWidget::updateSelection);

    connect(m_spinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_colorpicker,
            [=](int val) { m_selectedIndex = val; });

    m_vLayout->addWidget(m_spinboxLabel);
    m_vLayout->addWidget(m_spinbox);

    m_deletePresetButton = new QPushButton(tr("Delete"), this);

    connect(m_deletePresetButton,
            &QPushButton::pressed,
            this,
            &ColorPickerEditor::onDeletePreset);

    m_vLayout->addWidget(m_deletePresetButton);

    m_addPresetLabel = new QLabel(tr("Add Preset:"), this);
    m_colorInput = new QLineEdit(this);
    m_colorInput->setText(m_color.name(QColor::HexRgb));

    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorSelected,
            this,
            [=](QColor c) {
                m_color = c;
                m_colorInput->setText(m_color.name(QColor::HexRgb));
            });

    connect(m_colorInput, &QLineEdit::editingFinished, this, [=]() {
        if (QColor::isValidColor(m_colorInput->text())) {
            m_color = QColor(m_colorInput->text());
            m_colorWheel->setColor(m_color);
        }
        m_colorInput->setText(m_color.name(QColor::HexRgb));
    });

    m_addPresetButton = new QPushButton(tr("Add"), this);

    connect(m_addPresetButton,
            &QPushButton::pressed,
            this,
            &ColorPickerEditor::onAddPreset);

    m_vLayout->addWidget(m_addPresetLabel);
    m_vLayout->addWidget(m_colorInput);
    m_vLayout->addWidget(m_addPresetButton);

    m_hLayout->addLayout(m_vLayout);
}

void ColorPickerEditor::addPreset()
{
    ConfigHandler config;
    QVector<QColor> colors = config.userColors();

    if (colors.contains(m_color))
        return;

    colors << m_color;

    const int maxPresetsAllowed = 10;

    if (colors.size() > maxPresetsAllowed) {
        QMessageBox::critical(
          this, tr("Error"), tr("Unable to add more presets."));
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
          this, tr("Error"), tr("Unable to remove presets."));
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
    m_spinbox->setValue(1);
    m_colorpicker->updateWidget();
    m_spinbox->updateWidget();
}

void ColorPickerEditor::onDeletePreset()
{
    deletePreset();
    m_spinbox->setValue(1);
    m_colorpicker->updateWidget();
    m_spinbox->updateWidget();
}