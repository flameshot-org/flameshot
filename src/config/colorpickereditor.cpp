// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorpickereditor.h"
#include "src/utils/globalvalues.h"
#include "src/widgets/colorpickerwidget.h"
#include "src/widgets/spinbox.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>

ColorPickerEditor::ColorPickerEditor(QWidget* parent)
  : QWidget(parent)
{
    m_hLayout = new QHBoxLayout(this);

    m_colorWheel = new color_widgets::ColorWheel(this);
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

    m_vLayout->addWidget(m_spinboxLabel);
    m_vLayout->addWidget(m_spinbox);

    m_hLayout->addLayout(m_vLayout);

    setLayout(m_hLayout);
}