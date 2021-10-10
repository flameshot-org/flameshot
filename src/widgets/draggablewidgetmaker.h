// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>

class DraggableWidgetMaker : public QObject
{
    Q_OBJECT
public:
    DraggableWidgetMaker(QObject* parent = nullptr);

    void makeDraggable(QWidget* widget);
    bool hasMoved() const { return m_hasMoved; }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool m_isPressing = false;
    bool m_isDragging = false;
    bool m_hasMoved = false;
    QPoint m_mouseMovePos;
    QPoint m_mousePressPos;
};
