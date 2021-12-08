// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2018 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QDialog>

class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QLabel;
class ImageLabel;

class CaptureLauncher : public QDialog
{
    Q_OBJECT
public:
    explicit CaptureLauncher(QDialog* parent = nullptr);

private:
    void connectCaptureSlots();
    void disconnectCaptureSlots();

private slots:
    void startCapture();
    void startDrag();
    void captureTaken(QPixmap p, const QRect& selection);
    void captureFailed();

private:
    QSpinBox* m_delaySpinBox;
    QComboBox* m_captureType;
    QVBoxLayout* m_mainLayout;
    QPushButton* m_launchButton;
    QLabel* m_CaptureModeLabel;
    ImageLabel* m_imageLabel;
};
