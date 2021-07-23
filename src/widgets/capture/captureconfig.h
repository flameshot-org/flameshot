// SPDX-License-Identifier: GPL-3.0-or-later


#pragma once

#include <QObject>

namespace CaptureConfig {
    Q_NAMESPACE

    enum CaptureWindowMode {
        FullScreenAll,
        FullScreenCurrent,
        MaximizeWindow,
        DebugNonFullScreen
    };
    Q_ENUM_NS(CaptureWindowMode)
}
