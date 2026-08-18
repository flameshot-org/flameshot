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
            event->ignore();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}
