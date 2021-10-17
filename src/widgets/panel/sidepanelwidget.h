// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "color_wheel.hpp"
#include <QWidget>

class QVBoxLayout;
class QPushButton;
class QLabel;
class QLineEdit;
class ColorGrabWidget;
class QColorPickingEventFilter;
class QSlider;

constexpr int maxDrawThickness = 50;

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
    void onColorChanged(const QColor& c);
    void updateThickness(const int& t);

private slots:
    void startColorGrab();
    void onColorGrabFinished();
    void onColorGrabAborted();
    void onTemporaryColorUpdated(const QColor& color);

private:
    void finalizeGrab();
    void updateColorNoWheel(const QColor& c);

    bool eventFilter(QObject* obj, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;

    QVBoxLayout* m_layout;
    QPushButton* m_colorGrabButton;
    ColorGrabWidget* m_colorGrabber;
    color_widgets::ColorWheel* m_colorWheel;
    QLabel* m_colorLabel;
    QLineEdit* m_colorHex;
    QPixmap* m_pixmap;
    QColor m_color;
    QColor m_revertColor;
    QSlider* m_thicknessSlider;
    int m_thickness;
};
