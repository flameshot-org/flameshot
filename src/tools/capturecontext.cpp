// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturecontext.h"

QPixmap CaptureContext::selectedScreenshotArea() const
{
    if (selection.isNull()) {
        return screenshot;
    } else {
        return screenshot.copy(selection);
    }
}
