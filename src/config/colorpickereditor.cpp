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

    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setAlignment(Qt::AlignVCenter);

    m_spinboxLabel = new QLabel(tr("Select Preset:"), this);
    m_spinbox = new SpinBox(this);

    connect(m_spinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_colorpicker,
            &ColorPickerWidget::updateWidget);

    connect(m_spinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_colorpicker,
            [=](int val) { m_selectedIndex = val; });

    m_vLayout->addWidget(m_spinboxLabel);
    m_vLayout->addWidget(m_spinbox);

    m_deletePreset = new QPushButton(tr("Delete"), this);

    m_vLayout->addWidget(m_deletePreset);

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
            &ColorPickerEditor::addPreset);

    m_vLayout->addWidget(m_addPresetLabel);
    m_vLayout->addWidget(m_colorInput);
    m_vLayout->addWidget(m_addPresetButton);

    m_hLayout->addLayout(m_vLayout);

    setLayout(m_hLayout);
}

void ColorPickerEditor::addPreset()
{
    ConfigHandler config;
    QVector<QColor> colors = config.userColors();

    if (colors.contains(m_color))
        return;

    colors << m_color;

    // QString colorsString = QString();

    // for (int i = 0; i < colors.size(); ++i) {
    //     if (colors[i] == QColor()) {
    //         colorsString.append(QStringLiteral("picker"));
    //     } else {
    //         colorsString.append(colors[i].name(QColor::HexRgb));
    //     }
    //     if (i < colors.size() - 1) {
    //         colorsString.append(QStringLiteral(", "));
    //     }
    // }

    config.setUserColors(colors);
}