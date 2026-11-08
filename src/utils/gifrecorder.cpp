// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#include "gifrecorder.h"
#include "gifencoder.h"
#include "screengrabber.h"
#include "utils/abstractlogger.h"
#include "utils/desktopinfo.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QStandardPaths>
#include <QTemporaryFile>

#include <utility>

namespace {
constexpr int kStartDelayMs = 200;
constexpr int kMaxFailedGrabs = 3;
// Manual recordings stop automatically after this limit, so a forgotten
// recording cannot fill the disk.
constexpr int kHardDurationLimitSeconds = 600;
} // namespace

GifRecorder::GifRecorder(const QRect& region,
                         int fps,
                         int durationSeconds,
                         int loopCount,
                         int maxWidth,
                         QObject* parent)
  : QObject(parent)
  , m_region(region)
  , m_fps(qBound(1, fps, 50))
  , m_durationSeconds(qMax(0, durationSeconds))
  , m_loopCount(qMax(0, loopCount))
  , m_maxWidth(qMax(0, maxWidth))
  , m_frameDelay(qMax(1, qRound(1000.0 / m_fps)))
  , m_delayCentiseconds(qMax(2, qRound(100.0 / m_fps)))
  , m_scale(1.0)
  , m_state(State::Idle)
  , m_frameCount(0)
  , m_failedGrabs(0)
  , m_waylandWarningShown(false)
  , m_paused(false)
  , m_encoder(nullptr)
{
    const int width = qMax(1, region.width());
    const int height = qMax(1, region.height());
    if (m_maxWidth > 0 && width > m_maxWidth) {
        m_scale = qreal(m_maxWidth) / width;
    }
    m_outputSize = QSize(qMax(1, qRound(width * m_scale)),
                         qMax(1, qRound(height * m_scale)));

    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &GifRecorder::captureFrame);
}

GifRecorder::~GifRecorder()
{
    m_timer.stop();
    if (m_encoder) {
        m_encoder->close();
        delete m_encoder;
        m_encoder = nullptr;
    }
    if (!m_tempPath.isEmpty() && QFile::exists(m_tempPath)) {
        QFile::remove(m_tempPath);
    }
}

bool GifRecorder::isRecording() const
{
    return m_state == State::Recording;
}

void GifRecorder::setOverlayProvider(OverlayImageProvider image,
                                     OverlayVisibilityChecker canHide,
                                     OverlayVisibilitySetter setHidden)
{
    m_overlayImage = std::move(image);
    m_overlayCanHide = std::move(canHide);
    m_overlaySetHidden = std::move(setHidden);
}

void GifRecorder::start()
{
    if (m_state != State::Idle) {
        return;
    }
    m_state = State::Waiting;
    // Give the capture editor time to hide before the first frame is grabbed.
    QTimer::singleShot(kStartDelayMs, this, &GifRecorder::beginCapture);
}

void GifRecorder::beginCapture()
{
    if (m_state != State::Waiting) {
        return;
    }

    QString error;
    if (!createEncoder(error)) {
        fail(error);
        return;
    }

    m_frameCount = 0;
    m_failedGrabs = 0;
    m_elapsed.start();
    m_state = State::Recording;
    emit started();
    m_timer.start(m_frameDelay);
    captureFrame();
}

bool GifRecorder::createEncoder(QString& error)
{
    // Prefer the on-disk cache location over the temporary directory, which
    // is often a RAM-backed tmpfs and could fill up during long recordings.
    QString tempDir =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (tempDir.isEmpty()) {
        tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    if (tempDir.isEmpty() || !QDir().mkpath(tempDir)) {
        tempDir = QDir::tempPath();
    }

    QTemporaryFile file(QDir(tempDir).filePath("flameshot-gif-XXXXXX.gif"));
    file.setAutoRemove(false);
    if (!file.open()) {
        error = file.errorString();
        return false;
    }
    m_tempPath = file.fileName();
    file.close();

    m_encoder = new GifEncoder(
      m_outputSize.width(), m_outputSize.height(), m_loopCount);
    if (!m_encoder->open(m_tempPath)) {
        error = m_encoder->errorString();
        delete m_encoder;
        m_encoder = nullptr;
        QFile::remove(m_tempPath);
        m_tempPath.clear();
        return false;
    }
    return true;
}

void GifRecorder::captureFrame()
{
    if (m_state != State::Recording || !m_encoder || m_paused) {
        return;
    }

    QImage frame;
    QString error;
    if (!grabFrame(frame, error)) {
        ++m_failedGrabs;
        if (m_failedGrabs >= kMaxFailedGrabs) {
            fail(error);
        }
        return;
    }
    m_failedGrabs = 0;

    if (!m_encoder->addFrame(frame, m_delayCentiseconds)) {
        fail(m_encoder->errorString());
        return;
    }

    ++m_frameCount;
    emit progress(m_elapsed.elapsed(), m_frameCount);

    const qint64 limitSeconds = m_durationSeconds > 0
                                  ? m_durationSeconds
                                  : kHardDurationLimitSeconds;
    if (m_elapsed.elapsed() >= limitSeconds * 1000) {
        stop();
    }
}

bool GifRecorder::grabFrame(QImage& frame, QString& error)
{
    frame = QImage(m_outputSize, QImage::Format_RGB32);
    frame.fill(Qt::black);

    QPainter painter(&frame);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    bool captured = false;

    // Hide the drawing overlay while the desktop is grabbed, so its window
    // (and any stale compositor texture) never ends up in the frame. The
    // drawings are composited separately from the overlay model below.
    const bool hideOverlay = m_overlayCanHide && m_overlayCanHide();
    if (hideOverlay && m_overlaySetHidden) {
        m_overlaySetHidden(true);
    }

    DesktopInfo desktopInfo;
    const bool wayland = desktopInfo.waylandDetected();
    if (wayland && !m_waylandWarningShown) {
        m_waylandWarningShown = true;
        AbstractLogger::info()
          << tr("GIF recording on Wayland uses the screenshot portal and may "
               "be slow. Consider recording on X11 for a higher frame rate.");
    }

    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty() && !wayland) {
        // XRandR may expose no monitors on virtual/remote displays even
        // though the root window is valid. Fall back to a direct X11 grab.
        ScreenGrabber grabber;
        bool ok = false;
        QPixmap pixmap = grabber.grabEntireDesktop(ok);
        if (ok && !pixmap.isNull()) {
            const QRect source =
              m_region.intersected(QRect(QPoint(0, 0), pixmap.size()));
            if (!source.isEmpty()) {
                const QRect target(
                  qRound((source.x() - m_region.x()) * m_scale),
                  qRound((source.y() - m_region.y()) * m_scale),
                  qRound(source.width() * m_scale),
                  qRound(source.height() * m_scale));
                painter.drawPixmap(target, pixmap, source);
                captured = true;
            }
        }
    }

    for (QScreen* screen : screens) {
        const QRect screenGeometry = screen->geometry();
        const QRect intersection = m_region.intersected(screenGeometry);
        if (intersection.isEmpty()) {
            continue;
        }

        QPixmap pixmap;
        if (wayland) {
            ScreenGrabber grabber;
            bool ok = false;
            pixmap = grabber.grabScreen(screen, ok);
            if (!ok) {
                continue;
            }
        } else {
            pixmap = screen->grabWindow(0);
            // On X11 a grab can fail transiently right after a window is
            // hidden. Retry on the next frame instead of falling back to the
            // screenshot portal: a portal dialog would interrupt the
            // recording.
            if (pixmap.isNull()) {
                continue;
            }
        }
        if (pixmap.isNull()) {
            continue;
        }

        const qreal scaleX =
          pixmap.width() / qreal(qMax(1, screenGeometry.width()));
        const qreal scaleY =
          pixmap.height() / qreal(qMax(1, screenGeometry.height()));
        QRect source(
          qRound((intersection.x() - screenGeometry.x()) * scaleX),
          qRound((intersection.y() - screenGeometry.y()) * scaleY),
          qRound(intersection.width() * scaleX),
          qRound(intersection.height() * scaleY));
        source =
          source.intersected(QRect(0, 0, pixmap.width(), pixmap.height()));
        if (source.isEmpty()) {
            continue;
        }

        const QRect target(
          qRound((intersection.x() - m_region.x()) * m_scale),
          qRound((intersection.y() - m_region.y()) * m_scale),
          qRound(intersection.width() * m_scale),
          qRound(intersection.height() * m_scale));
        painter.drawPixmap(target, pixmap, source);
        captured = true;
    }

    if (hideOverlay && m_overlaySetHidden) {
        m_overlaySetHidden(false);
    }

    if (m_overlayImage) {
        const QImage overlay = m_overlayImage();
        if (!overlay.isNull()) {
            painter.drawImage(
              QRectF(0, 0, m_outputSize.width(), m_outputSize.height()),
              overlay,
              QRectF(0, 0, overlay.width(), overlay.height()));
        }
    }

    painter.end();
    if (!captured) {
        error = tr("Unable to capture the screen");
        return false;
    }
    return true;
}

void GifRecorder::stop()
{
    if (m_state == State::Waiting) {
        m_state = State::Idle;
        if (!m_tempPath.isEmpty()) {
            QFile::remove(m_tempPath);
            m_tempPath.clear();
        }
        emit cancelled();
        return;
    }
    if (m_state != State::Recording) {
        return;
    }

    m_timer.stop();
    m_paused = false;
    m_state = State::Idle;

    bool closed = true;
    QString error;
    if (m_encoder) {
        closed = m_encoder->close();
        error = m_encoder->errorString();
        delete m_encoder;
        m_encoder = nullptr;
    }
    if (!closed || !error.isEmpty()) {
        if (!m_tempPath.isEmpty()) {
            QFile::remove(m_tempPath);
        }
        m_tempPath.clear();
        emit failed(error.isEmpty() ? tr("Failed to write the GIF file")
                                    : error);
        return;
    }

    emit finished(m_tempPath, m_frameCount);
}

void GifRecorder::cancel()
{
    if (m_state == State::Idle) {
        return;
    }
    m_timer.stop();
    m_paused = false;
    m_state = State::Idle;
    if (m_encoder) {
        m_encoder->close();
        delete m_encoder;
        m_encoder = nullptr;
    }
    if (!m_tempPath.isEmpty()) {
        QFile::remove(m_tempPath);
        m_tempPath.clear();
    }
    emit cancelled();
}

void GifRecorder::pause()
{
    if (m_state == State::Recording && !m_paused) {
        m_paused = true;
        m_timer.stop();
    }
}

void GifRecorder::resume()
{
    if (m_state == State::Recording && m_paused) {
        m_paused = false;
        m_timer.start(m_frameDelay);
    }
}

void GifRecorder::setFps(int fps)
{
    fps = qBound(1, fps, 50);
    if (fps == m_fps) {
        return;
    }
    m_fps = fps;
    m_frameDelay = qMax(1, qRound(1000.0 / m_fps));
    m_delayCentiseconds = qMax(2, qRound(100.0 / m_fps));
    if (m_state == State::Recording && !m_paused) {
        m_timer.start(m_frameDelay);
    }
}

void GifRecorder::fail(const QString& error)
{
    m_timer.stop();
    m_paused = false;
    m_state = State::Idle;
    if (m_encoder) {
        m_encoder->close();
        delete m_encoder;
        m_encoder = nullptr;
    }
    if (!m_tempPath.isEmpty()) {
        QFile::remove(m_tempPath);
        m_tempPath.clear();
    }
    emit failed(error);
}
