// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
#include <QRect>
#include <QVector>

#include <optional>

/// Pure decision logic for which monitor a capture session is aimed at.
/// QtCore only, no widgets and no platform calls, so it is unit-testable
/// headless.
namespace MonitorFocus {

/// Index of the first geometry containing `globalPos`, or -1 when the point
/// lies in a gap between monitors or off the desktop entirely. Bottom and
/// right edges are exclusive; a shared edge resolves to the lowest index.
int monitorIndexAt(const QVector<QRect>& monitorGeometries,
                   const QPoint& globalPos);

/// Monitor a one-shot capture takes without asking: the one under the pointer
/// when following the cursor, or -1 to show the picker. -1 as well when the
/// pointer position is unknown (Wayland) or lies between monitors.
int monitorWithoutPicker(bool followCursor,
                         const std::optional<QPoint>& cursorPos,
                         const QVector<QRect>& monitorGeometries);

/// Tracks which monitor is armed, and freezes that choice once editing starts.
///
/// Input is source-agnostic: a Wayland per-window Enter, a mouse move, or a
/// QCursor::pos() poll all funnel into pointerAt().
class ActiveMonitorTracker
{
public:
    explicit ActiveMonitorTracker(int monitorCount, int initialMonitor = -1);

    /// The pointer is now over `monitorIndex`. An index that is negative or
    /// out of range means "unknown" and is ignored. Returns true iff
    /// activeMonitor() changed.
    bool pointerAt(int monitorIndex);

    /// The pointer left `monitorIndex`. Never changes activeMonitor(): during
    /// a crossing the neighbour's Enter does that, so there is no frame with
    /// nothing armed. Always returns false; callers may use it to assert
    /// Enter/Leave pairing.
    bool pointerLeft(int monitorIndex);

    /// First mouse press; latches permanently. A valid `monitorIndex` forces
    /// the latch onto that monitor, since the press says authoritatively where
    /// the pointer was even if an Enter was dropped. Ignored once latched.
    void beginEditing(int monitorIndex = -1);

    /// Whether a switch to `monitorIndex` is warranted right now.
    bool shouldSwitchTo(int monitorIndex) const;

    int activeMonitor() const { return m_active; }
    bool isLatched() const { return m_latched; }
    int monitorCount() const { return m_count; }

private:
    bool isValid(int monitorIndex) const;

    int m_count;
    int m_active;
    bool m_latched = false;
};

} // namespace MonitorFocus
