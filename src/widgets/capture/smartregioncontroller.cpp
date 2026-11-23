// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "smartregioncontroller.h"

#include <array>
#include <utility>

namespace {
// Minimum time between two lookups while the pointer moves.
constexpr int LOOKUP_INTERVAL_MS = 35;

// Some applications (Chromium, Electron, WebKit) build their accessibility
// tree lazily, so the first lookup only returns a coarse container. Repeat the
// lookup while the pointer rests, so the highlight settles on the element that
// a click will select before the user clicks. On Windows, these lookups also
// collect results that took longer than the GUI thread waits.
constexpr std::array<int, 5> REFINE_DELAYS_MS = { 150, 400, 1000, 2000, 3500 };

// A touchpad swipe produces a stream of small wheel events.
constexpr qint64 CONTINUOUS_STEP_INTERVAL_MS = 180;
}

SmartRegionController::SmartRegionController(
  std::unique_ptr<SmartRegionDetector> detector,
  QObject* parent)
  : QObject(parent)
  , m_detector(std::move(detector))
{
    m_rateLimit.setSingleShot(true);
    m_rateLimit.setInterval(LOOKUP_INTERVAL_MS);
    connect(&m_rateLimit, &QTimer::timeout, this, [this]() {
        if (m_pointer != m_lookedUpAt) {
            lookUp();
        }
    });

    m_refineTimer.setSingleShot(true);
    connect(&m_refineTimer, &QTimer::timeout, this, [this]() { refine(); });
}

SmartRegionController::~SmartRegionController() = default;

void SmartRegionController::pointerMoved(const QPoint& globalPosition)
{
    m_pointer = globalPosition;
    // Qt sends a synthetic move right before a press when it has not seen the
    // pointer yet. Looking up again here could silently replace the region the
    // user is looking at just before the click selects it.
    if (m_hasLookup && globalPosition == m_lookedUpAt) {
        return;
    }
    if (!m_rateLimit.isActive()) {
        lookUp();
    }
}

void SmartRegionController::step(int direction, bool continuousInput)
{
    if (direction == 0 || m_index < 0) {
        return;
    }
    if (continuousInput) {
        if (m_lastContinuousStep.isValid() &&
            m_lastContinuousStep.elapsed() < CONTINUOUS_STEP_INTERVAL_MS) {
            return;
        }
        m_lastContinuousStep.start();
    }

    const int index = qBound(0,
                             m_index + (direction > 0 ? 1 : -1),
                             static_cast<int>(m_regions.size()) - 1);
    m_preferred = m_regions.at(index);
    setIndex(index);
}

void SmartRegionController::suspend()
{
    m_rateLimit.stop();
    m_refineTimer.stop();
}

void SmartRegionController::clear()
{
    m_rateLimit.stop();
    m_refineTimer.stop();
    m_hasLookup = false;
    m_preferred = {};
    m_regions.clear();
    setIndex(-1);
}

QRect SmartRegionController::current() const
{
    return m_index >= 0 ? m_regions.at(m_index) : QRect();
}

bool SmartRegionController::elementDetectionAvailable() const
{
    return m_detector->elementDetectionAvailable();
}

void SmartRegionController::lookUp()
{
    m_lookedUpAt = m_pointer;
    m_hasLookup = true;
    setRegions(m_detector->regionsAt(m_pointer));
    m_rateLimit.start();
    m_refineAttempt = 0;
    m_refineTimer.start(REFINE_DELAYS_MS.front());
}

void SmartRegionController::refine()
{
    if (!m_hasLookup || m_pointer != m_lookedUpAt) {
        return;
    }
    // A lookup cut short by a slow application falls back to the window.
    // Only accept a hierarchy that is at least as deep as the current one,
    // so the highlight never jumps to a coarser region before a click.
    QList<QRect> regions = m_detector->regionsAt(m_pointer);
    if (regions.size() >= m_regions.size() && regions != m_regions) {
        setRegions(std::move(regions));
    }
    if (++m_refineAttempt < static_cast<int>(REFINE_DELAYS_MS.size())) {
        m_refineTimer.start(REFINE_DELAYS_MS.at(m_refineAttempt));
    }
}

void SmartRegionController::setRegions(QList<QRect> regions)
{
    const QRect previous = current();
    m_regions = std::move(regions);
    int index = m_preferred.isValid() ? m_regions.indexOf(m_preferred) : -1;
    if (index < 0) {
        m_preferred = {};
        index = m_regions.isEmpty() ? -1 : 0;
    }
    m_index = index;
    if (previous != current()) {
        emit currentChanged(previous, current());
    }
}

void SmartRegionController::setIndex(int index)
{
    const QRect previous = current();
    m_index = index;
    if (previous != current()) {
        emit currentChanged(previous, current());
    }
}
