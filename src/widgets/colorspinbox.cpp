// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#include "src/widgets/colorspinbox.h"
#include "src/utils/confighandler.h"

ColorSpinBox::ColorSpinBox(QWidget* parent)
  : QSpinBox(parent)
{
    initColorSpinbox();
}

int ColorSpinBox::valueFromText(const QString& text) const
{
    if (!QColor::isValidColor(text)) {
        return 1;
    }

    const QColor color = QColor(text);

    for (int i = 1; i < m_colorList.size(); ++i) {
        if (m_colorList.at(i) == color) {
            return i;
        }
    }

    return 1;
}

QString ColorSpinBox::textFromValue(int value) const
{
    return m_colorList[value].name(QColor::HexRgb);
}

void ColorSpinBox::initColorSpinbox()
{
    ConfigHandler config;
    m_colorList = config.userColors();

    setRange(1, m_colorList.size() - 1);
    setWrapping(true);
}

void ColorSpinBox::updateWidget()
{
    initColorSpinbox();
    update();
}
