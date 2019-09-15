// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "textconfig.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include <QFontDatabase>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

TextConfig::TextConfig(QWidget *parent) : QWidget(parent) {
    m_layout = new QVBoxLayout(this);

    QFontDatabase fontDB;
    QComboBox *fontsCB = new QComboBox();
    connect(fontsCB, &QComboBox::currentTextChanged,
            this, &TextConfig::fontFamilyChanged);
    fontsCB->addItems(fontDB.families());
    // TODO save family in config
    int index = fontsCB->findText(font().family());
    fontsCB->setCurrentIndex(index);

    QColor bgColor(palette().background().color());
    QString iconPrefix = ColorUtils::colorIsDark(bgColor) ?
                PathInfo::whiteIconPath() :
                PathInfo::blackIconPath();

    m_strikeOutButton = new QPushButton(
                QIcon(iconPrefix + "format_strikethrough.svg"), QLatin1String(""));
    m_strikeOutButton->setCheckable(true);
    connect(m_strikeOutButton, &QPushButton::clicked,
            this, &TextConfig::fontStrikeOutChanged);
    m_strikeOutButton->setToolTip(tr("StrikeOut"));

    m_underlineButton = new QPushButton(
                QIcon(iconPrefix + "format_underlined.svg"), QLatin1String(""));
    m_underlineButton->setCheckable(true);
    connect(m_underlineButton, &QPushButton::clicked,
            this, &TextConfig::fontUnderlineChanged);
    m_underlineButton->setToolTip(tr("Underline"));

    m_weightButton = new QPushButton(
                QIcon(iconPrefix + "format_bold.svg"), QLatin1String(""));
    m_weightButton->setCheckable(true);
    connect(m_weightButton, &QPushButton::clicked,
            this, &TextConfig::weightButtonPressed);
    m_weightButton->setToolTip(tr("Bold"));

    m_italicButton = new QPushButton(
                QIcon(iconPrefix + "format_italic.svg"), QLatin1String(""));
    m_italicButton->setCheckable(true);
    connect(m_italicButton, &QPushButton::clicked,
            this, &TextConfig::fontItalicChanged);
    m_italicButton->setToolTip(tr("Italic"));
    QHBoxLayout *modifiersLayout = new QHBoxLayout();

    m_layout->addWidget(fontsCB);
    modifiersLayout->addWidget(m_strikeOutButton);
    modifiersLayout->addWidget(m_underlineButton);
    modifiersLayout->addWidget(m_weightButton);
    modifiersLayout->addWidget(m_italicButton);
    m_layout->addLayout(modifiersLayout);
}

void TextConfig::setUnderline(const bool u) {
    m_underlineButton->setChecked(u);
}

void TextConfig::setStrikeOut(const bool s) {
    m_strikeOutButton->setChecked(s);
}

void TextConfig::setWeight(const int w) {
    m_weightButton->setChecked(static_cast<QFont::Weight>(w) == QFont::Bold);
}

void TextConfig::setItalic(const bool i) {
    m_italicButton->setChecked(i);
}

void TextConfig::weightButtonPressed(const bool w) {
    if (w) {
        emit fontWeightChanged(QFont::Bold);
    } else {
        emit fontWeightChanged(QFont::Normal);
    }
}
