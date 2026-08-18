// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QObject>

// QComboBox and QAbstractSpinBox react to the mouse wheel by default even
// when they don't have focus, so scrolling over one of these widgets
// inside Configuration silently changes that setting instead of leaving
// it alone. Installed once app-wide, this swallows wheel events on such
// widgets while they're unfocused; clicking into the widget first still
// allows scrolling to change its value as usual.
class WheelFocusGuard : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};
