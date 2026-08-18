// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 & Contributors

#include "monitorpickersurface.h"

#include <QPainter>

MonitorPickerSurface::MonitorPickerSurface(const QPixmap& background,
                                           QWidget* parent,
                                           Qt::WindowFlags flags)
  : QWidget(parent, flags)
  , m_background(background)
{
    if (m_background.isNull()) {
        setAttribute(Qt::WA_TranslucentBackground);
        setStyleSheet("QWidget { background-color: transparent; }");
    } else {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }
}

void MonitorPickerSurface::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    if (m_background.isNull()) {
        return; // translucent surface; children paint the content
    }
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_background);
}
