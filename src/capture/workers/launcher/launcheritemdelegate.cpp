// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "launcheritemdelegate.h"
#include "src/capture/widgets/capturebutton.h"
#include <QPainter>

LauncherItemDelegate::LauncherItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void LauncherItemDelegate::paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    const QRect &rect = option.rect;
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->save();
        painter->setPen(Qt::transparent);
        painter->setBrush(QPalette().highlight());
        painter->drawRect(rect.x(), rect.y(),
                          rect.width() -1, rect.height() -1);
        painter->restore();
    }
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();

    const int iconSide = CaptureButton::buttonBaseSize() * 1.3;
    const int halfIcon = iconSide/2;
    const int halfWidth = rect.width()/2;
    const int halfHeight = rect.height()/2;
    QSize size(iconSide, iconSide);
    QPixmap pixIcon = icon.pixmap(size).scaled(size, Qt::KeepAspectRatio);
    painter->drawPixmap(rect.x() + (halfWidth - halfIcon),
                        rect.y()+ (halfHeight/2 - halfIcon),
                        iconSide, iconSide, pixIcon);
    const QRect textRect(rect.x(), rect.y() + halfHeight,
                         rect.width(), halfHeight);
    painter->drawText(textRect, Qt::TextWordWrap | Qt::AlignHCenter,
                      index.data(Qt::DisplayRole).toString());
}

QSize LauncherItemDelegate::sizeHint(
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    const int size = CaptureButton::buttonBaseSize();
    return QSize(size * 3.2, size * 3.7);
}
