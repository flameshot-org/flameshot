// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "wheelfocusguard.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QCoreApplication>
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
            // An app-level filter runs before the widget's own event()
            // ever sees the event, so simply swallowing it here would stop
            // scrolling dead instead of letting the page continue - Qt's
            // usual "ignored wheel event bubbles to the parent" behavior
            // only kicks in once a widget's event() gets a chance to run.
            // Resending to the parent re-enters that normal delivery path
            // (and its own auto-propagation up the ancestor chain) so a
            // scroll started over this widget carries on scrolling.
            if (widget->parentWidget()) {
                QCoreApplication::sendEvent(widget->parentWidget(), event);
            }
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}
