// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QObject>

// QComboBox and QAbstractSpinBox react to the mouse wheel by default even
// when they don't have focus, which hijacks scrolling anywhere these
// widgets sit inside a QScrollArea (e.g. Configuration's General tab) -
// the user ends up silently changing a setting instead of scrolling past
// it. Installed once app-wide, this ignores wheel events on such widgets
// while they're unfocused so the event bubbles up to the parent scroll
// area instead; clicking into the widget first still allows scrolling to
// change its value as usual.
class WheelFocusGuard : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};
