// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#include "hovereventfilter.h"
#include <QEvent>

HoverEventFilter::HoverEventFilter(QObject* parent)
  : QObject(parent)
{}

bool HoverEventFilter::eventFilter(QObject* watched, QEvent* event)
{
    QEvent::Type t = event->type();
    switch (t) {
        case QEvent::Enter:
            emit hoverIn(watched);
            break;
        case QEvent::Leave:
            emit hoverOut(watched);
            break;
        default:
            break;
    }
    return false;
}
