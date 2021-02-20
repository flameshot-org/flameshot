// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on https://stackoverflow.com/a/53135675/964478

#pragma once

#include "capture/capturebutton.h"
#include <QPushButton>

class OrientablePushButton : public CaptureButton
{
    Q_OBJECT
public:
    enum Orientation
    {
        Horizontal,
        VerticalTopToBottom,
        VerticalBottomToTop
    };

    OrientablePushButton(QWidget* parent = nullptr);
    OrientablePushButton(const QString& text, QWidget* parent = nullptr);
    OrientablePushButton(const QIcon& icon,
                         const QString& text,
                         QWidget* parent = nullptr);

    QSize sizeHint() const;

    OrientablePushButton::Orientation orientation() const;
    void setOrientation(const OrientablePushButton::Orientation& orientation);

protected:
    void paintEvent(QPaintEvent* event);

private:
    Orientation m_orientation = Horizontal;
};
