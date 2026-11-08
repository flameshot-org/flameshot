// SPDX-License-Identifier: GPL-3.0-or-later
#include "macoscapturekeepalive.h"

#include <QTimer>

#import <ScreenCaptureKit/ScreenCaptureKit.h>

namespace {
// replayd idles out after ~29-30 minutes; ping well inside that window.
constexpr int kKeepAliveIntervalMs = 10 * 60 * 1000;

API_AVAILABLE(macos(12.3))
void pingReplayd()
{
    // Listing shareable content is the lightest request replayd serves. It
    // captures no pixels; the result is discarded.
    [SCShareableContent
      getShareableContentWithCompletionHandler:^(SCShareableContent*, NSError*) {
      }];
}
} // namespace

void startScreenCaptureKeepAlive(QObject* parent)
{
    // ScreenCaptureKit (and replayd's prompt) only exist on macOS 12.3+.
    if (@available(macOS 12.3, *)) {
        auto* timer = new QTimer(parent);
        timer->setInterval(kKeepAliveIntervalMs);
        QObject::connect(timer, &QTimer::timeout, timer, &pingReplayd);
        timer->start();
    }
}
