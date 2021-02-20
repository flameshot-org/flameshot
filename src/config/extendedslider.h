// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QSlider>
#include <QTimer>

class ExtendedSlider : public QSlider
{
    Q_OBJECT
public:
    explicit ExtendedSlider(QWidget* parent = nullptr);

    int mappedValue(int min, int max);
    void setMapedValue(int min, int val, int max);

signals:
    void modificationsEnded();

private slots:
    void updateTooltip();
    void fireTimer();

private:
    QTimer m_timer;
};
