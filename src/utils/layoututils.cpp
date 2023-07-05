// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "layoututils.h"

namespace layoututils
{

bool adjustRectInsideAnother(const QRect& parent, QRect& inner)
{
    if (inner.width() > parent.width() || inner.height() > parent.height()) {
        return false;
    }

    if (inner.left() < parent.left()) {
        inner.moveLeft(parent.left());
    }
    if (inner.right() > parent.right()) {
        inner.moveRight(parent.right());
    }
    if (inner.top() < parent.top()) {
        inner.moveTop(parent.top());
    }
    if (inner.bottom() > parent.bottom()) {
        inner.moveBottom(parent.bottom());
    }
    return true;
}

}