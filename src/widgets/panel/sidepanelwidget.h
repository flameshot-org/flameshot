// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "QtColorWidgets/color_wheel.hpp"
#include <QWidget>

class QVBoxLayout;
class QPushButton;
class QLabel;
class QLineEdit;
class ColorGrabWidget;
class QColorPickingEventFilter;
class QSlider;

constexpr int maxToolSize = 50;
constexpr int minSliderWidth = 100;

class SidePanelWidget : public QWidget
{
    Q_OBJECT

    friend class QColorPickingEventFilter;

public:
    explicit SidePanelWidget(QPixmap* p, QWidget* parent = nullptr);

signals:
    void colorChanged(const QColor& color);
    void toolSizeChanged(int size);
    void togglePanel();

public slots:
    void onToolSizeChanged(int tool);
    void onColorChanged(const QColor& color);

private slots:
    void startColorGrab();
    void onColorGrabFinished();
    void onColorGrabAborted();
    void onTemporaryColorUpdated(const QColor& color);

private:
    void finalizeGrab();
    void updateColorNoWheel(const QColor& color);

    bool eventFilter(QObject* obj, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;

    QVBoxLayout* m_layout;
    QPushButton* m_colorGrabButton;
    ColorGrabWidget* m_colorGrabber{};
    color_widgets::ColorWheel* m_colorWheel;
    QLabel* m_colorLabel;
    QLineEdit* m_colorHex;
    QPixmap* m_pixmap;
    QColor m_color;
    QColor m_revertColor;
    QSlider* m_toolSizeSlider;
    int m_toolSize{};
};
