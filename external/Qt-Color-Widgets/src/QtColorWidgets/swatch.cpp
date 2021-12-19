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
#include "QtColorWidgets/swatch.hpp"

#include <cmath>
#include <limits>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QStyleOption>
#include <QToolTip>

namespace color_widgets {

class Swatch::Private
{
public:
    ColorPalette palette;    ///< Palette with colors and related metadata
    int          selected;   ///< Current selection index (-1 for no selection)
    QSize        color_size; ///< Preferred size for the color squares
    ColorSizePolicy size_policy;
    QPen         border;
    int          forced_rows;
    int          forced_columns;
    bool         readonly;  ///< Whether the palette can be modified via user interaction

    QPoint  drag_pos;       ///< Point used to keep track of dragging
    int     drag_index;     ///< Index used by drags
    int     drop_index;     ///< Index for a requested drop
    QColor  drop_color;     ///< Dropped color
    bool    drop_overwrite; ///< Whether the drop will overwrite an existing color

    QSize   max_color_size;  ///< Mazimum size a color square can have

    bool show_clear_color = false;

    Swatch* owner;

    Private(Swatch* owner)
        : selected(-1),
          color_size(16,16),
          size_policy(Hint),
          border(Qt::black, 1),
          forced_rows(0),
          forced_columns(0),
          readonly(false),
          drag_index(-1),
          drop_index(-1),
          drop_overwrite(false),
          max_color_size(96, 128),
          owner(owner)
    {}

    /**
     * \brief Number of rows/columns in the palette
     */
    QSize rowcols()
    {
        int count = color_count();

        if ( count == 0 )
            return QSize();

        if ( forced_rows )
            return QSize(std::ceil( float(count) / forced_rows ), forced_rows);

        int columns = palette.columns();

        if ( forced_columns )
            columns = forced_columns;
        else if ( columns == 0 )
            columns = qMin(count, owner->width() / color_size.width());

        int rows = std::ceil( float(count) / columns );

        return QSize(columns, rows);
    }

    int color_count()
    {
        int count = palette.count();

        if ( show_clear_color )
            count++;

        return count;
    }

    /**
     * \brief Sets the drop properties
     */
    void dropEvent(QDropEvent* event)
    {
        // Find the output location
        drop_index = indexAt(event->pos());
        if ( drop_index == -1 )
            drop_index = palette.count();

        // Gather up the color
        if ( event->mimeData()->hasColor() )
        {
            drop_color = event->mimeData()->colorData().value<QColor>();
            drop_color.setAlpha(255);
        }
        else if ( event->mimeData()->hasText() )
        {
            drop_color = QColor(event->mimeData()->text());
        }

        drop_overwrite = false;
        QRectF drop_rect = indexRect(drop_index);
        if ( drop_index < palette.count() && drop_rect.isValid() )
        {
            // 1 column => vertical style
            if ( palette.columns() == 1 || forced_columns == 1 )
            {
                // Dragged to the last quarter of the size of the square, add after
                if ( event->posF().y() >= drop_rect.top() + drop_rect.height() * 3.0 / 4 )
                    drop_index++;
                // Dragged to the middle of the square, overwrite existing color
                else if ( event->posF().x() > drop_rect.top() + drop_rect.height() / 4 &&
                        ( event->dropAction() != Qt::MoveAction || event->source() != owner ) )
                    drop_overwrite = true;
            }
            else
            {
                // Dragged to the last quarter of the size of the square, add after
                if ( event->posF().x() >= drop_rect.left() + drop_rect.width() * 3.0 / 4 )
                    drop_index++;
                // Dragged to the middle of the square, overwrite existing color
                else if ( event->posF().x() > drop_rect.left() + drop_rect.width() / 4 &&
                        ( event->dropAction() != Qt::MoveAction || event->source() != owner ) )
                    drop_overwrite = true;
            }
        }

        owner->update();
    }

    /**
     * \brief Clears drop properties
     */
    void clearDrop()
    {
        drop_index = -1;
        drop_color = QColor();
        drop_overwrite = false;

        owner->update();
    }

    /**
     * \brief Actual size of a color square
     */
    QSizeF actualColorSize()
    {
        QSize rowcols = this->rowcols();
        if ( !rowcols.isValid() )
            return QSizeF();
        return actualColorSize(rowcols);
    }

    /**
     * \brief Actual size of a color square
     * \pre rowcols.isValid() and obtained via rowcols()
     */
    QSizeF actualColorSize(const QSize& rowcols)
    {
        return QSizeF (
            qMin(qreal(max_color_size.width()), qreal(owner->width()) / rowcols.width()),
            qMin(qreal(max_color_size.height()), qreal(owner->height()) / rowcols.height())
        );
    }


    /**
     * \brief Rectangle corresponding to the color at the given index
     * \pre rowcols.isValid() and obtained via rowcols()
     * \pre color_size obtained via rowlcols(rowcols)
     */
    QRectF indexRect(int index, const QSize& rowcols, const QSizeF& color_size)
    {
        if ( index == -1 )
            return QRectF();

        return QRectF(
            index % rowcols.width() * color_size.width(),
            index / rowcols.width() * color_size.height(),
            color_size.width(),
            color_size.height()
        );
    }
    /**
     * \brief Rectangle corresponding to the color at the given index
     */
    QRectF indexRect(int index)
    {
        QSize rc = rowcols();
        if ( index == -1 || !rc.isValid() )
            return QRectF();
        return indexRect(index, rc, actualColorSize(rc));
    }

    int indexAt(const QPoint& pt, bool mark_clear = false)
    {
        QSize rowcols = this->rowcols();
        if ( rowcols.isEmpty() )
            return -1;

        QSizeF color_size = actualColorSize(rowcols);

        QPoint point(
            pt.x() / color_size.width(),
            pt.y() / color_size.height()
        );

        if ( point.x() < 0 || point.x() >= rowcols.width() || point.y() < 0 || point.y() >= rowcols.height() )
            return -1;

        int index = point.y() * rowcols.width() + point.x();

        if ( mark_clear && index == palette.count() && show_clear_color )
            return -2;

        if ( index >= palette.count() )
            return -1;
        return index;
    }
};

Swatch::Swatch(QWidget* parent)
    : QWidget(parent), p(new Private(this))
{
    connect(&p->palette, &ColorPalette::colorsChanged, this, &Swatch::paletteModified);
    connect(&p->palette, &ColorPalette::colorAdded, this, &Swatch::paletteModified);
    connect(&p->palette, &ColorPalette::colorRemoved, this, &Swatch::paletteModified);
    connect(&p->palette, &ColorPalette::columnsChanged, this, (void(QWidget::*)())&QWidget::update);
    connect(&p->palette, &ColorPalette::colorsUpdated, this, (void(QWidget::*)())&QWidget::update);
    connect(&p->palette, &ColorPalette::colorChanged, [this](int index){
        if ( index == p->selected )
            Q_EMIT colorSelected( p->palette.colorAt(index) );
    });
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setAttribute(Qt::WA_Hover, true);
}

Swatch::~Swatch()
{
    delete p;
}

QSize Swatch::sizeHint() const
{
    QSize rowcols = p->rowcols();

    if ( !p->color_size.isValid() || !rowcols.isValid() )
        return QSize();

    return QSize(
        p->color_size.width()  * rowcols.width(),
        p->color_size.height() * rowcols.height()
    );
}

QSize Swatch::minimumSizeHint() const
{
    if ( p->size_policy != Hint )
        return sizeHint();
    return QSize();
}

const ColorPalette& Swatch::palette() const
{
    return p->palette;
}

ColorPalette& Swatch::palette()
{
    return p->palette;
}

int Swatch::selected() const
{
    return p->selected;
}

QColor Swatch::selectedColor() const
{
    return p->palette.colorAt(p->selected);
}

int Swatch::indexAt(const QPoint& pt)
{
    return p->indexAt(pt);
}

QColor Swatch::colorAt(const QPoint& pt)
{
    return p->palette.colorAt(indexAt(pt));
}

void Swatch::setPalette(const ColorPalette& palette)
{
    clearSelection();
    p->palette = palette;
    update();
    Q_EMIT paletteChanged(p->palette);
}

void Swatch::setSelected(int selected)
{
    if ( selected < 0 || selected >= p->palette.count() )
        selected = -1;

    if ( selected != p->selected )
    {
        Q_EMIT selectedChanged( p->selected = selected );
        if ( selected != -1 )
            Q_EMIT colorSelected( p->palette.colorAt(p->selected) );
    }
    update();
}

void Swatch::clearSelection()
{
    setSelected(-1);
}

void Swatch::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QSize rowcols = p->rowcols();

    QPainter painter(this);

    QStyleOptionFrame panel;
    panel.initFrom(this);
    panel.lineWidth = 1;
    panel.midLineWidth = 0;
    panel.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_Frame, &panel, &painter, this);

    if ( rowcols.isEmpty() )
        return;

    QSizeF color_size = p->actualColorSize(rowcols);
    QRect r = style()->subElementRect(QStyle::SE_FrameContents, &panel, this);
    painter.setClipRect(r);

    int count = p->palette.count();
    painter.setPen(p->border);
    for ( int y = 0, i = 0; i < count; y++ )
    {
        for ( int x = 0; x < rowcols.width() && i < count; x++, i++ )
        {
            painter.setBrush(p->palette.colorAt(i));
            painter.drawRect(p->indexRect(i, rowcols, color_size));
        }
    }

    if ( p->show_clear_color )
    {
        QRectF ir = p->indexRect(count, rowcols, color_size);
        painter.setBrush(QColor(255, 255, 255));
        painter.drawRect(ir);
        painter.setPen(QPen(QColor(0xa40000), qBound(1., 5., color_size.width()/3)));
        painter.setBrush(Qt::NoBrush);
        painter.setClipRect(ir, Qt::IntersectClip);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(ir.topLeft(), ir.bottomRight());
        painter.drawLine(ir.topRight(), ir.bottomLeft());
    }

    painter.setClipping(false);

    if ( p->drop_index != -1 )
    {
        QRectF drop_area = p->indexRect(p->drop_index, rowcols, color_size);
        if ( p->drop_overwrite )
        {
            painter.setBrush(p->drop_color);
            painter.setPen(QPen(Qt::gray));
            painter.drawRect(drop_area);
        }
        else if ( rowcols.width() == 1 )
        {
            // 1 column => vertical style
            painter.setPen(QPen(p->drop_color, 2));
            painter.setBrush(Qt::transparent);
            painter.drawLine(drop_area.topLeft(), drop_area.topRight());
        }
        else
        {
            painter.setPen(QPen(p->drop_color, 2));
            painter.setBrush(Qt::transparent);
            painter.drawLine(drop_area.topLeft(), drop_area.bottomLeft());
            // Draw also on the previous line when the first item of a line is selected
            if ( p->drop_index % rowcols.width() == 0 && p->drop_index != 0 )
            {
                drop_area = p->indexRect(p->drop_index-1, rowcols, color_size);
                drop_area.translate(color_size.width(), 0);
                painter.drawLine(drop_area.topLeft(), drop_area.bottomLeft());
            }
        }
    }

    if ( p->selected != -1 )
    {
        QRectF rect = p->indexRect(p->selected, rowcols, color_size);
        painter.setBrush(Qt::transparent);
        painter.setPen(QPen(Qt::darkGray, 2));
        painter.drawRect(rect);
        painter.setPen(QPen(Qt::gray, 2, Qt::DotLine));
        painter.drawRect(rect);
    }
}

void Swatch::keyPressEvent(QKeyEvent* event)
{
    if ( p->palette.count() == 0 )
        QWidget::keyPressEvent(event);

    int selected = p->selected;
    int count = p->palette.count();
    QSize rowcols = p->rowcols();
    int columns = rowcols.width();
    int rows = rowcols.height();
    switch ( event->key() )
    {
        default:
            QWidget::keyPressEvent(event);
            return;

        case Qt::Key_Left:
            if ( selected == -1 )
                selected = count - 1;
            else if ( selected > 0 )
                selected--;
            break;

        case Qt::Key_Right:
            if ( selected == -1 )
                selected = 0;
            else if ( selected < count - 1 )
                selected++;
            break;

        case Qt::Key_Up:
            if ( selected == -1 )
                selected = count - 1;
            else if ( selected >= columns )
                selected -= columns;
            break;

        case Qt::Key_Down:
            if ( selected == -1 )
                selected = 0;
            else if ( selected < count - columns )
                selected += columns;
            break;

        case Qt::Key_Home:
            if ( event->modifiers() & Qt::ControlModifier )
                selected = 0;
            else
                selected -= selected % columns;
            break;

        case Qt::Key_End:
            if ( event->modifiers() & Qt::ControlModifier )
                selected = count - 1;
            else
                selected += columns - (selected % columns) - 1;
            break;

        case Qt::Key_Delete:
            removeSelected();
            return;

        case Qt::Key_Backspace:
            if (selected != -1 && !p->readonly )
            {
                p->palette.eraseColor(selected);
                if ( p->palette.count() == 0 )
                    selected = -1;
                else
                    selected = qMax(selected - 1, 0);
            }
            break;

        case Qt::Key_PageUp:
            if ( selected == -1 )
                selected = 0;
            else
                selected = selected % columns;
            break;
        case Qt::Key_PageDown:
            if ( selected == -1 )
            {
                selected = count - 1;
            }
            else
            {
                selected = columns * (rows-1) + selected % columns;
                if ( selected >= count )
                    selected -= columns;
            }
            break;
    }
    setSelected(selected);
}

void Swatch::removeSelected()
{
    if (p->selected != -1 && !p->readonly )
    {
        int selected = p->selected;
        p->palette.eraseColor(p->selected);
        setSelected(qMin(selected, p->palette.count() - 1));
    }
}

void Swatch::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        int index = p->indexAt(event->pos(), true);
        setSelected(index);
        p->drag_pos = event->pos();
        p->drag_index = index;
        if ( index == -2 )
            Q_EMIT clicked(-1, event->modifiers());
        else if ( index != -1 )
            Q_EMIT clicked(index, event->modifiers());
    }
    else if ( event->button() == Qt::RightButton )
    {
        int index = p->indexAt(event->pos(), true);

        if ( index == -2 )
            Q_EMIT rightClicked(-1, event->modifiers());
        else if ( index != -1 )
            Q_EMIT rightClicked(index, event->modifiers());
    }
}

void Swatch::mouseMoveEvent(QMouseEvent *event)
{
    if ( p->drag_index != -1 &&  (event->buttons() & Qt::LeftButton) &&
        (p->drag_pos - event->pos()).manhattanLength() >= QApplication::startDragDistance() )
    {
        QColor color = p->palette.colorAt(p->drag_index);

        QPixmap preview(24,24);
        preview.fill(color);

        QMimeData *mimedata = new QMimeData;
        mimedata->setColorData(color);
        mimedata->setText(p->palette.nameAt(p->drag_index));

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimedata);
        drag->setPixmap(preview);
        Qt::DropActions actions = Qt::CopyAction;
        if ( !p->readonly )
            actions |= Qt::MoveAction;
        drag->exec(actions);
    }
}

void Swatch::mouseReleaseEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        p->drag_index = -1;
    }
}

void Swatch::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        int index = p->indexAt(event->pos(), true);

        if ( index == -2 )
            Q_EMIT doubleClicked(-1, event->modifiers());
        else if ( index != -1 )
            Q_EMIT doubleClicked(index, event->modifiers());
    }
}

void Swatch::wheelEvent(QWheelEvent* event)
{
    if ( event->angleDelta().y() < 0 )
        p->selected = qMin(p->selected + 1, p->palette.count() - 1);
    else if ( p->selected == -1 )
            p->selected = p->palette.count() - 1;
    else if ( p->selected > 0 )
        p->selected--;
    setSelected(p->selected);
}

void Swatch::dragEnterEvent(QDragEnterEvent *event)
{
    if ( p->readonly )
        return;

    p->dropEvent(event);

    if ( p->drop_color.isValid() && p->drop_index != -1 )
    {
        if ( event->proposedAction() == Qt::MoveAction && event->source() == this )
            event->setDropAction(Qt::MoveAction);
        else
            event->setDropAction(Qt::CopyAction);

        event->accept();
    }
}

void Swatch::dragMoveEvent(QDragMoveEvent* event)
{
    if ( p->readonly )
        return;
    p->dropEvent(event);
}

void Swatch::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    p->clearDrop();
}

void Swatch::dropEvent(QDropEvent *event)
{
    if ( p->readonly )
        return;

    QString name;

    // Gather up the color
    if ( event->mimeData()->hasColor() && event->mimeData()->hasText() )
            name = event->mimeData()->text();

    // Not a color, discard
    if ( !p->drop_color.isValid() || p->drop_index == -1 )
        return;

    p->dropEvent(event);

    // Move unto self
    if ( event->dropAction() == Qt::MoveAction && event->source() == this )
    {
        // Not moved => noop
        if ( p->drop_index != p->drag_index && p->drop_index != p->drag_index + 1 )
        {
            // Erase the old color
            p->palette.eraseColor(p->drag_index);
            if ( p->drop_index > p->drag_index )
                p->drop_index--;
            p->selected = p->drop_index;
            // Insert the dropped color
            p->palette.insertColor(p->drop_index, p->drop_color, name);
        }
    }
    // Move into a color cell
    else if ( p->drop_overwrite )
    {
        p->palette.setColorAt(p->drop_index, p->drop_color, name);
    }
    // Insert the dropped color
    else
    {
        p->palette.insertColor(p->drop_index, p->drop_color, name);
    }

    // Finalize
    event->accept();
    p->drag_index = -1;
    p->clearDrop();
}

void Swatch::paletteModified()
{
    if ( p->selected >= p->palette.count() )
        clearSelection();

    if ( p->size_policy != Hint )
    {
        QSize size_hint = sizeHint();

        if ( size_hint.isValid() )
        {
            if ( p->size_policy == Minimum )
                setMinimumSize(size_hint);
            else if ( p->size_policy == Fixed )
                setFixedSize(size_hint);
        }
    }

    update();
}

QSize Swatch::colorSize() const
{
    return p->color_size;
}

void Swatch::setColorSize(const QSize& colorSize)
{
    if ( p->color_size != colorSize )
        Q_EMIT colorSizeChanged(p->color_size = colorSize);
}

QSize Swatch::maxColorSize() const
{
    return p->max_color_size;
}

void Swatch::setMaxColorSize(const QSize& colorSize)
{
    if ( p->max_color_size != colorSize )
        Q_EMIT maxColorSizeChanged(p->max_color_size = colorSize);
}

Swatch::ColorSizePolicy Swatch::colorSizePolicy() const
{
    return p->size_policy;
}

void Swatch::setColorSizePolicy(ColorSizePolicy colorSizePolicy)
{
    if ( p->size_policy != colorSizePolicy )
    {
        setMinimumSize(0,0);
        setFixedSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
        Q_EMIT colorSizePolicyChanged(p->size_policy = colorSizePolicy);
        paletteModified();
    }
}

int Swatch::forcedColumns() const
{
    return p->forced_columns;
}

int Swatch::forcedRows() const
{
    return p->forced_rows;
}

void Swatch::setForcedColumns(int forcedColumns)
{
    if ( forcedColumns <= 0 )
        forcedColumns = 0;

    if ( forcedColumns != p->forced_columns )
    {
        Q_EMIT forcedColumnsChanged(p->forced_columns = forcedColumns);
        Q_EMIT forcedRowsChanged(p->forced_rows = 0);
    }
}

void Swatch::setForcedRows(int forcedRows)
{
    if ( forcedRows <= 0 )
        forcedRows = 0;

    if ( forcedRows != p->forced_rows )
    {
        Q_EMIT forcedColumnsChanged(p->forced_columns = 0);
        Q_EMIT forcedRowsChanged(p->forced_rows = forcedRows);
    }
}

bool Swatch::readOnly() const
{
    return p->readonly;
}

void Swatch::setReadOnly(bool readOnly)
{
    if ( readOnly != p->readonly )
    {
        Q_EMIT readOnlyChanged(p->readonly = readOnly);
        setAcceptDrops(!p->readonly);
    }
}

bool Swatch::event(QEvent* event)
{
    if(event->type() == QEvent::ToolTip)
    {
        QHelpEvent* help_ev = static_cast<QHelpEvent*>(event);
        int index = p->indexAt(help_ev->pos(), true);
        if ( index == -2 )
        {
            QToolTip::showText(help_ev->globalPos(), tr("Clear Color"), this, p->indexRect(index).toRect());
            event->accept();
        }
        else if ( index != -1 )
        {
            QColor color = p->palette.colorAt(index);
            QString name = p->palette.nameAt(index);
            QString message = color.name();
            if ( !name.isEmpty() )
                message = tr("%1 (%2)").arg(name).arg(message);
            message = "<tt style='background-color:"+color.name()+";color:"+color.name()+";'>MM</tt> "+message.toHtmlEscaped();
            QToolTip::showText(help_ev->globalPos(), message, this,
                               p->indexRect(index).toRect());
            event->accept();
        }
        else
        {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }

    return QWidget::event(event);
}

QPen Swatch::border() const
{
    return p->border;
}

void Swatch::setBorder(const QPen& border)
{
    if ( border != p->border )
    {
        p->border = border;
        Q_EMIT borderChanged(border);
        update();
    }
}

bool Swatch::showClearColor() const
{
    return p->show_clear_color;
}

void Swatch::setShowClearColor(bool show)
{
    if ( show != p->show_clear_color )
    {
        Q_EMIT showClearColorChanged(p->show_clear_color = show);
        update();
    }
}

} // namespace color_widgets
