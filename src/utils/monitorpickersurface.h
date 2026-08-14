// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 & Contributors

#pragma once

#include <QPixmap>
#include <QWidget>

// Window surface that paints a full-window background pixmap (used by the
// monitor picker on Wayland, where fullscreen windows cannot be translucent:
// the compositor composites them as opaque and the transparent area renders
// black, so the frozen desktop of the monitor is painted instead).
//
// With a null pixmap the widget is translucent and children paint the content
// (used on X11, where a small window can be moved freely).
class MonitorPickerSurface : public QWidget
{
public:
    explicit MonitorPickerSurface(const QPixmap& background,
                                  QWidget* parent = nullptr,
                                  Qt::WindowFlags flags = Qt::WindowFlags());

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap m_background;
};
