// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

#include "gifdrawingtool.h"

#include <QColor>
#include <QRect>
#include <QString>
#include <QWidget>

class QKeyEvent;
class QCloseEvent;
class QLabel;
class QPushButton;
class QButtonGroup;
class QSpinBox;
class QLineEdit;

/**
 * Small always-on-top panel shown while a GIF is being recorded. It lets the
 * user stop the recording or discard it and displays the elapsed time.
 *
 * In interactive mode it also hosts the drawing tools used over the recording.
 */
class GifRecordingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GifRecordingWidget(const QRect& region,
                                int durationSeconds,
                                QWidget* parent = nullptr);

    void setElapsed(qint64 elapsedMs, int frameCount);
    void setInteractive(bool interactive);
    void clearToolSelection();
    void setFps(int fps);
    QString textInput() const;
    void focusTextInput();

signals:
    void stopRequested();
    void cancelRequested();
    void toolSelected(GifDrawingTool tool);
    void undoRequested();
    void clearRequested();
    void colorChanged(const QColor& color);
    void widthChanged(int width);
    void fpsChanged(int fps);
    void textSubmitted();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void placeNearRegion(const QRect& region);
    QString formatDuration(qint64 milliseconds) const;
    void updateColorButton();
    void updateHint();

    QLabel* m_statusLabel;
    QLabel* m_hintLabel;
    int m_durationSeconds;
    QRect m_region;
    bool m_interactive = false;

    QWidget* m_toolsRow = nullptr;
    QButtonGroup* m_toolGroup = nullptr;
    QPushButton* m_colorButton = nullptr;
    QSpinBox* m_widthSpinBox = nullptr;
    QSpinBox* m_fpsSpinBox = nullptr;
    QLineEdit* m_textEdit = nullptr;
    QColor m_color;
};
