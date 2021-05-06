// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on https://stackoverflow.com/a/53135675/964478

#include "orientablepushbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>

OrientablePushButton::OrientablePushButton(QWidget* parent)
  : CaptureButton(parent)
{}

OrientablePushButton::OrientablePushButton(const QString& text, QWidget* parent)
  : CaptureButton(text, parent)
{}

OrientablePushButton::OrientablePushButton(const QIcon& icon,
                                           const QString& text,
                                           QWidget* parent)
  : CaptureButton(icon, text, parent)
{}

QSize OrientablePushButton::sizeHint() const
{
    QSize sh = QPushButton::sizeHint();

    if (m_orientation != OrientablePushButton::Horizontal) {
        sh.transpose();
    }

    return sh;
}

void OrientablePushButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);

    if (m_orientation == OrientablePushButton::VerticalTopToBottom) {
        painter.rotate(90);
        painter.translate(0, -1 * width());
        option.rect = option.rect.transposed();
    }

    else if (m_orientation == OrientablePushButton::VerticalBottomToTop) {
        painter.rotate(-90);
        painter.translate(-1 * height(), 0);
        option.rect = option.rect.transposed();
    }

    painter.drawControl(QStyle::CE_PushButton, option);
}

OrientablePushButton::Orientation OrientablePushButton::orientation() const
{
    return m_orientation;
}

void OrientablePushButton::setOrientation(
  const OrientablePushButton::Orientation& orientation)
{
    m_orientation = orientation;
}
