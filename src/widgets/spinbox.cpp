// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "src/widgets/spinbox.h"
#include "src/utils/confighandler.h"

SpinBox::SpinBox(QWidget* parent)
  : QSpinBox(parent)
{
    initSpinbox();
}

int SpinBox::valueFromText(const QString& text) const
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

QString SpinBox::textFromValue(int value) const
{
    return m_colorList[value].name(QColor::HexRgb);
}

void SpinBox::initSpinbox()
{
    ConfigHandler config;
    m_colorList = config.userColors();

    setRange(1, m_colorList.size() - 1);
    setWrapping(true);
}

void SpinBox::updateWidget()
{
    initSpinbox();
    update();
}
