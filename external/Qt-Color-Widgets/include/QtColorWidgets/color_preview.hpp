/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2014 Calle Laakkonen
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
#ifndef COLOR_PREVIEW_HPP
#define COLOR_PREVIEW_HPP

#include "colorwidgets_global.hpp"

#include <QWidget>

namespace color_widgets {

/**
 * Simple widget that shows a preview of a color
 */
class QCP_EXPORT ColorPreview : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true)
    Q_PROPERTY(QColor comparisonColor READ comparisonColor WRITE setComparisonColor NOTIFY comparisonColorChanged DESIGNABLE true)
    Q_PROPERTY(DisplayMode display_mode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged DESIGNABLE true)
    Q_PROPERTY(QBrush background READ background WRITE setBackground NOTIFY backgroundChanged DESIGNABLE true)
    Q_PROPERTY(bool drawFrame READ drawFrame WRITE setDrawFrame NOTIFY drawFrameChanged DESIGNABLE true)
    Q_ENUMS(DisplayMode)
public:
    enum DisplayMode
    {
        NoAlpha,    ///< Show current color with no transparency
        AllAlpha,   ///< show current color with transparency
        SplitAlpha, ///< Show both solid and transparent side by side
        SplitColor, ///< Show current and comparison colors side by side
        SplitColorReverse, ///< Like Split color but swapped
    };
    Q_ENUMS(DisplayMode)

    explicit ColorPreview(QWidget *parent = 0);
    ~ColorPreview();

    /// Get the background visible under transparent colors
    QBrush background() const;

    /// Change the background visible under transparent colors
    void setBackground(const QBrush &bk);

    /// Get color display mode
    DisplayMode displayMode() const;

    /// Set how transparent colors are handled
    void setDisplayMode(DisplayMode dm);

    /// Get current color
    QColor color() const;

    /// Get the comparison color
    QColor comparisonColor() const;

    QSize sizeHint () const Q_DECL_OVERRIDE;

    void paint(QPainter &painter, QRect rect) const;

    /// Whether to draw a frame around the color
    bool drawFrame() const;
    void setDrawFrame(bool);
    
public Q_SLOTS:
    /// Set current color
    void setColor(const QColor &c);

    /// Set the comparison color
    void setComparisonColor(const QColor &c);

Q_SIGNALS:
    /// Emitted when the user clicks on the widget
    void clicked();

    /// Emitted on setColor
    void colorChanged(QColor);

    void comparisonColorChanged(QColor);
    void displayModeChanged(DisplayMode);
    void backgroundChanged(const QBrush&);
    void drawFrameChanged(bool);
protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;

private:
    class Private;
    Private * const p;
};

} // namespace color_widgets
Q_DECLARE_METATYPE(color_widgets::ColorPreview::DisplayMode)

#endif // COLOR_PREVIEW_HPP
