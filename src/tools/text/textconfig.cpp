// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "textconfig.h"
#include "src/utils/colorutils.h"
#include "src/utils/confighandler.h"
#include "src/utils/pathinfo.h"
#include <QComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QPushButton>

TextConfig::TextConfig(QWidget* parent)
  : QWidget(parent)
  , m_layout(nullptr)
  , m_fontsCB(nullptr)
  , m_strikeOutButton(nullptr)
  , m_underlineButton(nullptr)
  , m_weightButton(nullptr)
  , m_italicButton(nullptr)
{
    m_layout = new QVBoxLayout(this);

    QFontDatabase fontDB;
    m_fontsCB = new QComboBox();
    connect(m_fontsCB,
            &QComboBox::currentTextChanged,
            this,
            &TextConfig::fontFamilyChanged);
    m_fontsCB->addItems(fontDB.families());
    setFontFamily(ConfigHandler().fontFamily());

    QString iconPrefix = ColorUtils::colorIsDark(palette().windowText().color())
                           ? PathInfo::blackIconPath()
                           : PathInfo::whiteIconPath();

    m_strikeOutButton = new QPushButton(
      QIcon(iconPrefix + "format_strikethrough.svg"), QLatin1String(""));
    m_strikeOutButton->setCheckable(true);
    connect(m_strikeOutButton,
            &QPushButton::clicked,
            this,
            &TextConfig::fontStrikeOutChanged);
    m_strikeOutButton->setToolTip(tr("StrikeOut"));

    m_underlineButton = new QPushButton(
      QIcon(iconPrefix + "format_underlined.svg"), QLatin1String(""));
    m_underlineButton->setCheckable(true);
    connect(m_underlineButton,
            &QPushButton::clicked,
            this,
            &TextConfig::fontUnderlineChanged);
    m_underlineButton->setToolTip(tr("Underline"));

    m_weightButton =
      new QPushButton(QIcon(iconPrefix + "format_bold.svg"), QLatin1String(""));
    m_weightButton->setCheckable(true);
    connect(m_weightButton,
            &QPushButton::clicked,
            this,
            &TextConfig::weightButtonPressed);
    m_weightButton->setToolTip(tr("Bold"));

    m_italicButton = new QPushButton(QIcon(iconPrefix + "format_italic.svg"),
                                     QLatin1String(""));
    m_italicButton->setCheckable(true);
    connect(m_italicButton,
            &QPushButton::clicked,
            this,
            &TextConfig::fontItalicChanged);
    m_italicButton->setToolTip(tr("Italic"));
    QHBoxLayout* modifiersLayout = new QHBoxLayout();

    m_layout->addWidget(m_fontsCB);
    modifiersLayout->addWidget(m_strikeOutButton);
    modifiersLayout->addWidget(m_underlineButton);
    modifiersLayout->addWidget(m_weightButton);
    modifiersLayout->addWidget(m_italicButton);
    m_layout->addLayout(modifiersLayout);
}

void TextConfig::setFontFamily(const QString& fontFamily)
{
    m_fontsCB->setCurrentIndex(
      m_fontsCB->findText(fontFamily.isEmpty() ? font().family() : fontFamily));
}

void TextConfig::setUnderline(const bool u)
{
    m_underlineButton->setChecked(u);
}

void TextConfig::setStrikeOut(const bool s)
{
    m_strikeOutButton->setChecked(s);
}

void TextConfig::setWeight(const int w)
{
    m_weightButton->setChecked(static_cast<QFont::Weight>(w) == QFont::Bold);
}

void TextConfig::setItalic(const bool i)
{
    m_italicButton->setChecked(i);
}

void TextConfig::weightButtonPressed(const bool w)
{
    if (w) {
        emit fontWeightChanged(QFont::Bold);
    } else {
        emit fontWeightChanged(QFont::Normal);
    }
}
