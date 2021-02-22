// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class LoadSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit LoadSpinner(QWidget* parent = nullptr);

    void setColor(const QColor& c);
    void setWidth(int w);
    void setHeight(int h);
    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void rotate();

private:
    QColor m_color;
    QTimer* m_timer;

    int m_startAngle = 0;
    int m_span = 180;
    bool m_growing;

    QRect m_frame;
    void updateFrame();
};
