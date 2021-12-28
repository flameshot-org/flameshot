// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "src/widgets/spinbox.h"
#include "src/utils/confighandler.h"

SpinBox::SpinBox(QWidget* parent)
  : QSpinBox(parent)
{
    ConfigHandler config;
    m_colorList = config.userColors();

    this->setRange(1, m_colorList.size() - 1);
    this->setWrapping(true);
}

int SpinBox::valueFromText(const QString& text) const
{
    if (!QColor::isValidColor(text)) {
        return 0;
    }

    const QColor color = QColor(text);

    for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorList.at(i) == color) {
            return i;
        }
    }

    return 0;
}

QString SpinBox::textFromValue(int value) const
{
    return m_colorList[value].name(QColor::HexRgb);
}
