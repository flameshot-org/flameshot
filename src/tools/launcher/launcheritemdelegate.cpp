// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "launcheritemdelegate.h"
#include "src/utils/globalvalues.h"
#include <QPainter>

LauncherItemDelegate::LauncherItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{}

void LauncherItemDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    const QRect& rect = option.rect;
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->save();
        painter->setPen(Qt::transparent);
        painter->setBrush(QPalette().highlight());
        painter->drawRect(
          rect.x(), rect.y(), rect.width() - 1, rect.height() - 1);
        painter->restore();
    }
    auto icon = index.data(Qt::DecorationRole).value<QIcon>();

    const int iconSide = static_cast<int>(GlobalValues::buttonBaseSize() * 1.3);
    const int halfIcon = iconSide / 2;
    const int halfWidth = rect.width() / 2;
    const int halfHeight = rect.height() / 2;
    QSize size(iconSide, iconSide);
    QPixmap pixIcon = icon.pixmap(size).scaled(
      size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(rect.x() + (halfWidth - halfIcon),
                        rect.y() + (halfHeight / 2 - halfIcon),
                        iconSide,
                        iconSide,
                        pixIcon);
    const QRect textRect(
      rect.x(), rect.y() + halfHeight, rect.width(), halfHeight);
    painter->drawText(textRect,
                      Qt::TextWordWrap | Qt::AlignHCenter,
                      index.data(Qt::DisplayRole).toString());
}

QSize LauncherItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    const int size = GlobalValues::buttonBaseSize();
    return { static_cast<int>(size * 3.2), static_cast<int>(size * 3.7) };
}
