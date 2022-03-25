// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2018 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class CaptureLauncher;
}
QT_END_NAMESPACE

class CaptureLauncher : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureLauncher(QDialog* parent = nullptr);
    ~CaptureLauncher();

private:
    Ui::CaptureLauncher* ui;
    void connectCaptureSlots() const;
    void disconnectCaptureSlots() const;

private slots:
    void startCapture();
    void onCaptureTaken(QPixmap p);
    void onCaptureFailed();
};
