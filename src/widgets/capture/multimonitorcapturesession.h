// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/capturerequest.h"
#include "utils/monitorfocus.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QVector>

class CaptureWidget;
class QTimer;

/// Runs a capture that covers every display at once.
///
/// Wayland gives a client no global cursor position and no control over where
/// its surfaces land, so "follow the pointer to another display" cannot be
/// done by moving one window. Instead every display gets its own fullscreen
/// CaptureWidget and the one holding pointer focus is armed. The first press
/// latches that display and the rest are discarded.
class MultiMonitorCaptureSession : public QObject
{
    Q_OBJECT
public:
    explicit MultiMonitorCaptureSession(QObject* parent = nullptr);

    /// Builds and shows one widget per display, sharing a single screenshot.
    /// Returns false if the grab failed, in which case nothing is shown.
    bool start(const CaptureRequest& request);

    CaptureWidget* armedWidget() const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    /// The armed display changed; the argument may be null.
    void armedChanged(CaptureWidget* armed);
    /// The user committed to a display. Only that widget survives.
    void latched(CaptureWidget* widget);

private:
    void handlePointerEntered(int monitorIndex);
    void handleEditingStarted(int monitorIndex);
    void handleWidgetDestroyed(QObject* dying);
    void applyArmedState();
    void discardAllExcept(int keepIndex);

    QList<QPointer<CaptureWidget>> m_widgets;
    QVector<QRect> m_geometries;
    MonitorFocus::ActiveMonitorTracker m_tracker{ 0 };
    QTimer* m_pollTimer = nullptr;
    bool m_tearingDown = false;
};
