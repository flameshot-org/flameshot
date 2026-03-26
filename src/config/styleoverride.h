// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Jeremy Borgman <borgman.jeremy@pm.me>
//
// Created by jeremy on 9/24/20.

#pragma once

#include <QObject>
#include <QProxyStyle>

class StyleOverride : public QProxyStyle
{
    Q_OBJECT
public:
    int styleHint(StyleHint hint,
                  const QStyleOption* option = Q_NULLPTR,
                  const QWidget* widget = Q_NULLPTR,
                  QStyleHintReturn* returnData = Q_NULLPTR) const;
};
