// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Flameshot Contributors

#include "qrcontroller.h"

#ifdef ENABLE_QR_DECODER

#include "qrresultwidget.h"
#include "utils/confighandler.h"
#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

// Maximum time (in ms) a QR decode may take before the result is discarded.
static constexpr int QR_DECODE_TIMEOUT_MS = 5000;

// Minimum size of a version-1 QR code in modules (21x21).
static constexpr int MIN_QR_SELECTION_SIZE = 21;

QrController::QrController(QWidget* parent)
    : QObject(parent)
    , m_parentWidget(parent)
    , m_resultWidget(new QrResultWidget(parent))
    , m_scanJobId(0)
{
    connect(m_resultWidget,
            &QrResultWidget::dismissed,
            this,
            &QrController::hideResult);
    connect(m_resultWidget,
            &QrResultWidget::openUrlRequested,
            this,
            [this](const QUrl&) {
                hideResult();
                emit requestCloseCapture();
            });
}

void QrController::handleSelectionSettled(const QRect& geometry,
                                         const QPixmap& selectedArea)
{
    if (!ConfigHandler().enableQrCode()) {
        return;
    }

    if (geometry.width() < MIN_QR_SELECTION_SIZE ||
        geometry.height() < MIN_QR_SELECTION_SIZE ||
        selectedArea.isNull()) {
        return;
    }

    // Invalidate any ongoing scan job so previous asynchronous decodes are ignored
    const int jobId = ++m_scanJobId;
    m_currentScanningGeometry = geometry;

    // Run async decode on a worker thread
    auto* watcher = new QFutureWatcher<QrDecodeResult>(this);

    // Watchdog timer in case of pathological image or hung decode
    auto* watchdog = new QTimer(watcher);
    watchdog->setSingleShot(true);
    watchdog->setInterval(QR_DECODE_TIMEOUT_MS);
    connect(watchdog, &QTimer::timeout, watcher, &QObject::deleteLater);
    watchdog->start();

    connect(watcher,
            &QFutureWatcher<QrDecodeResult>::finished,
            this,
            [this, watcher, watchdog, jobId, geometry]() {
                const QrDecodeResult result = watcher->result();
                watchdog->stop();
                watcher->deleteLater();

                // Stale result check: ignore if a newer scan started or geometry changed
                if (jobId != m_scanJobId ||
                    geometry != m_currentScanningGeometry) {
                    return;
                }

                if (result.found && m_resultWidget) {
                    m_resultWidget->showResult(result.content, geometry);
                }
            });

    watcher->setFuture(QtConcurrent::run(
      [selectedArea]() { return QrDecoder::decode(selectedArea); }));
}

void QrController::handleSelectionDragging()
{
    // Invalidate pending scan and hide visible result
    m_scanJobId++;
    hideResult();
}

void QrController::handleSelectionHidden()
{
    m_scanJobId++;
    hideResult();
}

bool QrController::handleEscape()
{
    if (isVisible()) {
        hideResult();
        return true;
    }
    return false;
}

void QrController::handleMousePress(const QPoint& pos)
{
    if (isVisible() && m_resultWidget &&
        !m_resultWidget->geometry().contains(pos)) {
        hideResult();
    }
}

bool QrController::isVisible() const
{
    return m_resultWidget && m_resultWidget->isVisible();
}

void QrController::hideResult()
{
    if (m_resultWidget && m_resultWidget->isVisible()) {
        m_resultWidget->hideResult();
    }
}

#endif // ENABLE_QR_DECODER
