// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "smartregiondetector.h"

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include <memory>

// Tracks which detected region is highlighted before a selection exists.
//
// The highlighted region is the single source of truth: it is what the
// capture widget paints and what a click selects. It only changes when the
// pointer really moves, when the wheel walks the hierarchy, or when a
// follow-up lookup at the same resting position finds a different hierarchy.
// In each case currentChanged() is emitted so the widget can repaint.
class SmartRegionController : public QObject
{
    Q_OBJECT
public:
    explicit SmartRegionController(
      std::unique_ptr<SmartRegionDetector> detector,
      QObject* parent = nullptr);
    ~SmartRegionController() override;

    // Feed the pointer position, in global logical coordinates. Lookups are
    // rate-limited, and repeated positions are ignored.
    void pointerMoved(const QPoint& globalPosition);

    // Walk the hierarchy: a positive step moves toward the containing window,
    // a negative one toward the deepest element. Continuous (touchpad) input
    // is rate-limited so a single swipe does not skip several levels.
    void step(int direction, bool continuousInput);

    // Keep the current region until the next pointer move, e.g. while a
    // mouse button is held, so the region pressed is the one selected.
    void suspend();

    // Forget the current regions, e.g. while a selection is being drawn.
    void clear();

    // The highlighted region in global logical coordinates, or a null rect.
    QRect current() const;

    bool elementDetectionAvailable() const;

signals:
    void currentChanged(const QRect& previous, const QRect& current);

private:
    void lookUp();
    void refine();
    void setRegions(QList<QRect> regions);
    void setIndex(int index);

    std::unique_ptr<SmartRegionDetector> m_detector;
    QList<QRect> m_regions;
    int m_index = -1;
    // Region picked with the wheel. It stays highlighted while the pointer
    // moves within it, so the user can move between its children.
    QRect m_preferred;

    QPoint m_pointer;
    QPoint m_lookedUpAt;
    bool m_hasLookup = false;

    QTimer m_rateLimit;
    QTimer m_refineTimer;
    int m_refineAttempt = 0;
    QElapsedTimer m_lastContinuousStep;
};
