// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Flameshot Contributors

#pragma once

#ifdef ENABLE_QR_DECODER

#include "utils/qrdecoder.h"
#include <QObject>
#include <QPixmap>
#include <QRect>

class QWidget;
class QrResultWidget;

/**
 * @brief Coordinates QR code scanning lifecycle for capture selections.
 *
 * Encapsulates asynchronous background decoding, watchdog timeout management,
 * stale result filtering, and result panel interactions.
 */
class QrController : public QObject
{
    Q_OBJECT

public:
    explicit QrController(QWidget* parent);
    ~QrController() override = default;

    /**
     * Called when selection geometry settles and is ready to scan.
     */
    void handleSelectionSettled(const QRect& geometry, const QPixmap& selectedArea);

    /**
     * Called when selection begins dragging or resizing.
     */
    void handleSelectionDragging();

    /**
     * Called when selection is cleared or hidden.
     */
    void handleSelectionHidden();

    /**
     * Called when Escape key is pressed. Returns true if QR result was visible
     * and dismissed (meaning the key event was consumed).
     */
    bool handleEscape();

    /**
     * Called on mouse press. Dismisses result panel if clicked outside its area.
     */
    void handleMousePress(const QPoint& pos);

    /**
     * Check if result panel is currently visible.
     */
    bool isVisible() const;

    /**
     * Hide the result panel and stop auto-hide timers.
     */
    void hideResult();

signals:
    /**
     * Emitted when a QR action (such as opening URL in browser) completes
     * the capture session and requests closing the capture overlay.
     */
    void requestCloseCapture();

private:
    QWidget* m_parentWidget;
    QrResultWidget* m_resultWidget;
    QRect m_currentScanningGeometry;
    int m_scanJobId;
};

#endif // ENABLE_QR_DECODER
