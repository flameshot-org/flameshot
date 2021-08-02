/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef COLOR_DELEGATE_HPP
#define COLOR_DELEGATE_HPP

#include "colorwidgets_global.hpp"

#include <QStyledItemDelegate>

namespace color_widgets {


class QCP_EXPORT ReadOnlyColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setSizeHintForColor(const QSize& size_hint);
    const QSize& sizeHintForColor() const;

protected:
    void paintItem(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index, const QBrush& brush) const;
private:
    QSize size_hint{24, 16};
};

/**
    Delegate to use a ColorSelector in a color list
*/
class QCP_EXPORT ColorDelegate : public ReadOnlyColorDelegate
{
    Q_OBJECT
public:
    using ReadOnlyColorDelegate::ReadOnlyColorDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;

protected:
    bool eventFilter(QObject * watched, QEvent * event) Q_DECL_OVERRIDE;

private slots:
    void close_editor();
    void color_changed();
};

} // namespace color_widgets

#endif // COLOR_DELEGATE_HPP
