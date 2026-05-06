// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "iconconfig.h"

#include <QComboBox>
#include <QList>
#include <QPair>
#include <QVBoxLayout>
#include <QtGlobal>

IconConfig::IconConfig(QWidget* parent)
  : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_symbolCB(new QComboBox())
{
    const QList<QPair<QString, QString>> symbols = {
        { tr("Check"), QStringLiteral("✓") },
        { tr("Cross"), QStringLiteral("✕") },
        { tr("Warning"), QStringLiteral("⚠") },
        { tr("Info"), QStringLiteral("i") },
        { tr("Question"), QStringLiteral("?") },
        { tr("Star"), QStringLiteral("★") },
        { tr("Heart"), QStringLiteral("♥") },
        { tr("Arrow"), QStringLiteral("➜") },
    };

    for (const auto& symbol : symbols) {
        m_symbolCB->addItem(QStringLiteral("%1  %2").arg(symbol.second, symbol.first),
                            symbol.second);
    }

    connect(m_symbolCB,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            [this](int index) {
                emit symbolChanged(m_symbolCB->itemData(index).toString());
            });

    m_symbolCB->setToolTip(tr("Select an icon to add to the capture"));
    m_layout->addWidget(m_symbolCB);
}

void IconConfig::setSymbol(const QString& symbol)
{
    const int index = m_symbolCB->findData(symbol);
    if (index >= 0) {
        m_symbolCB->setCurrentIndex(index);
    }
}
