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
#include "QtColorWidgets/color_delegate.hpp"
#include "QtColorWidgets/color_selector.hpp"
#include "QtColorWidgets/color_dialog.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>


void color_widgets::ReadOnlyColorDelegate::paintItem(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index,
    const QBrush& brush
) const
{
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QWidget* widget = option.widget;
        opt.showDecorationSelected = true;
        QStyle *style = widget ? widget->style() : QApplication::style();
        QRect geom = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
        opt.text = "";

        QStyleOptionFrame panel;
        panel.initFrom(option.widget);
        if (option.widget->isEnabled())
            panel.state = QStyle::State_Enabled;
        panel.rect = geom;
        panel.lineWidth = 2;
        panel.midLineWidth = 0;
        panel.state |= QStyle::State_Sunken;

        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        style->drawPrimitive(QStyle::PE_Frame, &panel, painter, nullptr);
        QRect r = style->subElementRect(QStyle::SE_FrameContents, &panel, nullptr);
        painter->fillRect(r, brush);
}



void color_widgets::ReadOnlyColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        paintItem(painter, option, index, index.data().value<QColor>());
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize color_widgets::ReadOnlyColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
        return size_hint;
    return QStyledItemDelegate::sizeHint(option, index);
}

void color_widgets::ReadOnlyColorDelegate::setSizeHintForColor(const QSize& size_hint)
{
    this->size_hint = size_hint;
}

const QSize& color_widgets::ReadOnlyColorDelegate::sizeHintForColor() const
{
    return size_hint;
}


QWidget *color_widgets::ColorDelegate::createEditor(
    QWidget *parent,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        ColorDialog *editor = new ColorDialog(const_cast<QWidget*>(parent));
        connect(editor, &QDialog::accepted, this, &ColorDelegate::close_editor);
        connect(editor, &ColorDialog::colorSelected, this, &ColorDelegate::color_changed);
        return editor;
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void color_widgets::ColorDelegate::color_changed()
{
    ColorDialog *editor = qobject_cast<ColorDialog*>(sender());
    emit commitData(editor);
}

void color_widgets::ColorDelegate::close_editor()
{
    ColorDialog *editor = qobject_cast<ColorDialog*>(sender());
    emit closeEditor(editor);
}

void color_widgets::ColorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

    if ( index.data().type() == QVariant::Color )
    {
        ColorDialog *selector = qobject_cast<ColorDialog*>(editor);
        selector->setColor(qvariant_cast<QColor>(index.data()));
        return;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void color_widgets::ColorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        ColorDialog *selector = qobject_cast<ColorDialog *>(editor);
        model->setData(index, QVariant::fromValue(selector->color()));
        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}

void color_widgets::ColorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    if ( index.data().type() == QVariant::Color )
    {
        return;
    }


    QStyledItemDelegate::updateEditorGeometry(editor, option, index);

}

bool color_widgets::ColorDelegate::eventFilter(QObject * watched, QEvent * event)
{
    if ( event->type() == QEvent::Hide )
    {
        if ( auto editor = qobject_cast<ColorDialog*>(watched) )
        {
            emit closeEditor(editor, QAbstractItemDelegate::NoHint);
            return false;
        }
    }
    return QStyledItemDelegate::eventFilter(watched, event);
}
