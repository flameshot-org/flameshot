// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class QObject;

/**
 * @brief Keep macOS's screen-capture service (replayd) from idling out.
 *
 * replayd is stopped by launchd after ~30 idle minutes
 * (JETSAM_REASON_MEMORY_IDLE_EXIT). When it starts again it has forgotten the
 * user's answer to the "bypass the system private window picker" prompt, so
 * the next capture shows that prompt again. A cheap ScreenCaptureKit request
 * every few minutes keeps replayd busy enough that it is not stopped.
 *
 * The timer is parented to @p parent and stops with it.
 */
void startScreenCaptureKeepAlive(QObject* parent);
