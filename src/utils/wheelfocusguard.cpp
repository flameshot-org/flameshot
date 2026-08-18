// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "wheelfocusguard.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QEvent>
#include <QWidget>

bool WheelFocusGuard::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Wheel) {
        auto* widget = qobject_cast<QWidget*>(watched);
        bool isScrollSensitive =
          qobject_cast<QComboBox*>(widget) ||
          qobject_cast<QAbstractSpinBox*>(widget);
        if (isScrollSensitive && !widget->hasFocus()) {
            // Deliberately just swallow the event rather than trying to
            // forward it to the enclosing scroll area: Qt's "ignored wheel
            // event walks up to the parent" behavior only fires for real,
            // spontaneous hardware events, so manually resending it here
            // (tried and reverted - see git history) ends up re-entering
            // Qt's own event machinery in ways that misbehave. Simply not
            // reacting while unfocused is the actual requirement; clicking
            // into the widget first still lets the wheel change its value.
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}
