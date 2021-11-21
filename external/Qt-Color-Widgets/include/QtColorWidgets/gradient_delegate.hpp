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

#ifndef COLOR_WIDGETS_GRADIENT_DELEGATE_HPP
#define COLOR_WIDGETS_GRADIENT_DELEGATE_HPP


#include <QStyledItemDelegate>
#include <QPainter>

#include "QtColorWidgets/gradient_editor.hpp"

namespace color_widgets {

/**
 * \brief Item delegate to edits gradients
 *
 * In order to make it work, return as edit data from the model a QBrush with a gradient
 */
class QCP_EXPORT GradientDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const Q_DECL_OVERRIDE
    {
        QVariant data = index.data(Qt::EditRole);
        if ( data.canConvert<QBrush>() )
        {
            QBrush brush = data.value<QBrush>();
            if ( brush.gradient() )
            {
                GradientEditor* editor = new GradientEditor(parent);
                editor->setStops(brush.gradient()->stops());
                return editor;
            }
        }

        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void setModelData(QWidget * widget, QAbstractItemModel * model, const QModelIndex & index) const Q_DECL_OVERRIDE
    {
        if ( GradientEditor* editor = qobject_cast<GradientEditor*>(widget) )
            model->setData(index, QBrush(editor->gradient()), Qt::EditRole);
        else
            QStyledItemDelegate::setModelData(widget, model, index);
    }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const Q_DECL_OVERRIDE
    {
        QVariant display_data = index.data(Qt::DisplayRole);
        QVariant gradient_data = display_data.isValid() ? display_data : index.data(Qt::EditRole);
        if ( gradient_data.canConvert<QBrush>() )
        {
            QBrush brush = gradient_data.value<QBrush>();
            if ( brush.gradient() )
            {
                QBrush background;
                background.setTexture(QPixmap(QStringLiteral(":/color_widgets/alphaback.png")));
                painter->fillRect(option.rect, background);

                QLinearGradient g(option.rect.topLeft(), option.rect.topRight());
                g.setStops(brush.gradient()->stops());
                painter->fillRect(option.rect, g);

                if ( option.state & QStyle::State_Selected )
                {
                    int border = 2;
                    painter->setBrush(Qt::transparent);
                    painter->setPen(QPen(option.palette.highlight(), border));
                    painter->drawRect(option.rect.adjusted(border/2, border/2, -border/2, -border/2));
                }
                return;
            }
        }

        QStyledItemDelegate::paint(painter, option, index);
    }
};

} // namespace color_widgets

#endif // COLOR_WIDGETS_GRADIENT_DELEGATE_HPP
