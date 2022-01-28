// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021-2022 Jeremy Borgman & Contributors

#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class InfoWindow;
}
QT_END_NAMESPACE

class InfoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget* parent = nullptr);
    ~InfoWindow();

private:
    Ui::InfoWindow* ui;

protected:
    void keyPressEvent(QKeyEvent* event);

private slots:
    void on_CopyInfoButton_clicked();
};

QString generateKernelString();
