// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "color_wheel.hpp"
#include <QWidget>

class QVBoxLayout;
class QPushButton;
class QLabel;
class ColorGrabWidget;
class QColorPickingEventFilter;
class QSlider;

class SidePanelWidget : public QWidget
{
    Q_OBJECT

    friend class QColorPickingEventFilter;

public:
    explicit SidePanelWidget(QPixmap* p, QWidget* parent = nullptr);

signals:
    void colorChanged(const QColor& c);
    void thicknessChanged(int t);
    void togglePanel();

public slots:
    void updateColor(const QColor& c);
    void updateThickness(const int& t);

private slots:
    void updateColorNoWheel(const QColor& c);
    void updateCurrentThickness(int value);

private slots:
    void startColorGrab();
    void onColorGrabFinished();
    void onColorGrabAborted();
    void onColorUpdated(const QColor& color);

private:
    void updateGrabButton(const bool activated);
    void finalizeGrab();

    QVBoxLayout* m_layout;
    QPushButton* m_colorGrabButton;
    ColorGrabWidget* m_colorGrabber;
    color_widgets::ColorWheel* m_colorWheel;
    QLabel* m_colorLabel;
    QPixmap* m_pixmap;
    QColor m_color;
    QColor m_revertColor;
    QSlider* m_thicknessSlider;
    int m_thickness;
};
