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
#ifndef COLOR_WIDGETS_SWATCH_HPP
#define COLOR_WIDGETS_SWATCH_HPP

#include <QWidget>
#include <QPen>
#include "color_palette.hpp"

namespace color_widgets {

/**
 * \brief A widget drawing a palette
 */
class QCP_EXPORT Swatch : public QWidget
{
    Q_OBJECT

    /**
     * \brief Palette shown by the widget
     */
    Q_PROPERTY(const ColorPalette& palette READ palette WRITE setPalette NOTIFY paletteChanged)
    /**
     * \brief Currently selected color (-1 if no color is selected)
     */
    Q_PROPERTY(int selected READ selected WRITE setSelected NOTIFY selectedChanged)

    /**
     * \brief Preferred size for a color square
     */
    Q_PROPERTY(QSize colorSize READ colorSize WRITE setColorSize NOTIFY colorSizeChanged)

    Q_PROPERTY(ColorSizePolicy colorSizePolicy READ colorSizePolicy WRITE setColorSizePolicy NOTIFY colorSizePolicyChanged)

    /**
     * \brief Border around the colors
     */
    Q_PROPERTY(QPen border READ border WRITE setBorder NOTIFY borderChanged)

    /**
     * \brief Forces the Swatch to display that many rows of colors
     *
     * If there are too few elements, the widget will display less than this
     * many rows.
     *
     * A value of0 means that the number of rows is automatic.
     *
     * \note Conflicts with forcedColumns
     */
    Q_PROPERTY(int forcedRows READ forcedRows WRITE setForcedRows NOTIFY forcedRowsChanged)

    /**
     * \brief Forces the Swatch to display that many columns of colors
     *
     * If there are too few elements, the widget will display less than this
     * many columns.
     *
     * A value of 0 means that the number of columns is automatic.
     *
     * \note Conflicts with forcedRows
     */
    Q_PROPERTY(int forcedColumns READ forcedColumns WRITE setForcedColumns NOTIFY forcedColumnsChanged)

    /**
     * \brief Whether the palette can be modified via user interaction
     * \note Even when this is \b false, it can still be altered programmatically
     */
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly NOTIFY readOnlyChanged)


    /**
     * \brief Maximum size a color square can have
     */
    Q_PROPERTY(QSize maxColorSize READ maxColorSize WRITE setMaxColorSize NOTIFY maxColorSizeChanged)

    /**
     * \brief Whether to show an extra color to perform a "clear" operation.
     *
     * Clicking on this extra pseudo-color will emit signals like clicked() etc with an index of -1.
     */
    Q_PROPERTY(bool showClearColor READ showClearColor WRITE setShowClearColor NOTIFY showClearColorChanged)

public:
    enum ColorSizePolicy
    {
        Hint,    ///< The size is just a hint
        Minimum, ///< Can expand but not contract
        Fixed    ///< Must be exactly as specified
    };
    Q_ENUMS(ColorSizePolicy)

    Swatch(QWidget* parent = 0);
    ~Swatch();

    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    const ColorPalette& palette() const;
    ColorPalette& palette();
    int selected() const;
    /**
     * \brief Color at the currently selected index
     */
    QColor selectedColor() const;

    /**
     * \brief Color index at the given position within the widget
     * \param p Point in local coordinates
     * \returns -1 if the position doesn't represent any color
     */
    int indexAt(const QPoint& p);

    /**
     * \brief Color at the given position within the widget
     * \param p Point in local coordinates
     */
    QColor colorAt(const QPoint& p);

    QSize colorSize() const;
    QSize maxColorSize() const;
    ColorSizePolicy colorSizePolicy() const;
    QPen border() const;

    int forcedRows() const;
    int forcedColumns() const;

    bool readOnly() const;

    bool showClearColor() const;

public Q_SLOTS:
    void setPalette(const ColorPalette& palette);
    void setSelected(int selected);
    void clearSelection();
    void setColorSize(const QSize& colorSize);
    void setMaxColorSize(const QSize& colorSize);
    void setColorSizePolicy(ColorSizePolicy colorSizePolicy);
    void setBorder(const QPen& border);
    void setForcedRows(int forcedRows);
    void setForcedColumns(int forcedColumns);
    void setReadOnly(bool readOnly);
    /**
     * \brief Remove the currently seleceted color
     **/
    void removeSelected();
    void setShowClearColor(bool show);

Q_SIGNALS:
    void paletteChanged(const ColorPalette& palette);
    void selectedChanged(int selected);
    void colorSelected(const QColor& color);
    void colorSizeChanged(const QSize& colorSize);
    void maxColorSizeChanged(const QSize& colorSize);
    void colorSizePolicyChanged(ColorSizePolicy colorSizePolicy);
    void doubleClicked(int index, Qt::KeyboardModifiers modifiers);
    void rightClicked(int index, Qt::KeyboardModifiers modifiers);
    void clicked(int index, Qt::KeyboardModifiers modifiers);
    void forcedRowsChanged(int forcedRows);
    void forcedColumnsChanged(int forcedColumns);
    void readOnlyChanged(bool readOnly);
    void borderChanged(const QPen& border);
    void showClearColorChanged(bool show);

protected:
    bool event(QEvent* event) Q_DECL_OVERRIDE;

    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

protected Q_SLOTS:
    /**
     * \brief Connected to the internal palette object to keep eveything consistent
     */
    void paletteModified();

private:
    class Private;
    Private* p;
};


} // namespace color_widgets
#endif // COLOR_WIDGETS_SWATCH_HPP
