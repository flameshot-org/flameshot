// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QString>
#include <QTimer>

class GifEncoder;
class QImage;
class QScreen;

/**
 * Captures a region of the desktop at a fixed frame rate and writes it to a
 * temporary GIF file. The file is streamed frame by frame, so the memory usage
 * does not depend on the recording length.
 */
class GifRecorder : public QObject
{
    Q_OBJECT

public:
    GifRecorder(const QRect& region,
                int fps,
                int durationSeconds,
                int loopCount,
                int maxWidth,
                QObject* parent = nullptr);
    ~GifRecorder() override;

    QSize outputSize() const { return m_outputSize; }
    bool isRecording() const;

public slots:
    // Starts recording after a short delay, so that the capture widget can be
    // hidden before the first frame is grabbed.
    void start();
    void stop();
    void cancel();
    // Temporarily stops grabbing frames, e.g. while a modal dialog is shown
    // and must not appear in the recording.
    void pause();
    void resume();
    // Changes the frame rate of an ongoing recording. Frames already written
    // keep their delay; following frames use the new one.
    void setFps(int fps);

signals:
    void started();
    void progress(qint64 elapsedMs, int frameCount);
    void finished(const QString& tempFilePath, int frameCount);
    void failed(const QString& error);
    void cancelled();

private slots:
    void beginCapture();
    void captureFrame();

private:
    enum class State
    {
        Idle,
        Waiting,
        Recording,
    };

    bool createEncoder(QString& error);
    bool grabFrame(QImage& frame, QString& error);
    void fail(const QString& error);

    QRect m_region;
    int m_fps;
    int m_durationSeconds;
    int m_loopCount;
    int m_maxWidth;
    int m_frameDelay;
    int m_delayCentiseconds;

    QSize m_outputSize;
    qreal m_scale;

    State m_state;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    int m_frameCount;
    int m_failedGrabs;
    bool m_waylandWarningShown;
    bool m_paused;

    GifEncoder* m_encoder;
    QString m_tempPath;
};
