// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorfocus.h"

namespace MonitorFocus {

int monitorIndexAt(const QVector<QRect>& monitorGeometries,
                   const QPoint& globalPos)
{
    // QRect::contains() already treats bottom/right as exclusive of the next
    // monitor's first pixel, and first-match resolves shared edges to the
    // lowest index.
    for (int i = 0; i < monitorGeometries.size(); ++i) {
        if (monitorGeometries.at(i).contains(globalPos)) {
            return i;
        }
    }
    return -1;
}

int monitorWithoutPicker(bool followCursor,
                         const std::optional<QPoint>& cursorPos,
                         const QVector<QRect>& monitorGeometries)
{
    if (!followCursor || !cursorPos) {
        return -1;
    }
    return monitorIndexAt(monitorGeometries, *cursorPos);
}

ActiveMonitorTracker::ActiveMonitorTracker(int monitorCount, int initialMonitor)
  : m_count(monitorCount)
  , m_active(initialMonitor)
{}

bool ActiveMonitorTracker::isValid(int monitorIndex) const
{
    return monitorIndex >= 0 && monitorIndex < m_count;
}

bool ActiveMonitorTracker::pointerAt(int monitorIndex)
{
    if (!shouldSwitchTo(monitorIndex)) {
        return false;
    }
    m_active = monitorIndex;
    return true;
}

bool ActiveMonitorTracker::pointerLeft(int monitorIndex)
{
    Q_UNUSED(monitorIndex)
    return false;
}

void ActiveMonitorTracker::beginEditing(int monitorIndex)
{
    if (m_latched) {
        return;
    }
    if (isValid(monitorIndex)) {
        m_active = monitorIndex;
    }
    m_latched = true;
}

bool ActiveMonitorTracker::shouldSwitchTo(int monitorIndex) const
{
    return isValid(monitorIndex) && !m_latched && monitorIndex != m_active;
}

} // namespace MonitorFocus
