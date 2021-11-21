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
#ifndef COLOR_WIDGETS_COLOR_PALETTE_HPP
#define COLOR_WIDGETS_COLOR_PALETTE_HPP

#include <QColor>
#include <QString>
#include <QVector>
#include <QObject>
#include <QPair>
#include <QPixmap>
#include "colorwidgets_global.hpp"

namespace color_widgets {

class QCP_EXPORT ColorPalette : public QObject
{
    Q_OBJECT

    /**
     * \brief The list of colors
     */
    Q_PROPERTY(QVector<value_type> colors READ colors WRITE setColors NOTIFY colorsChanged)
    /**
     * \brief Name of the palette
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /**
     * \brief Number of colors to display in a row, if 0 unspecified
     */
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    /**
     * \brief Number of colors
     */
    Q_PROPERTY(int count READ count)
    /**
     * \brief Name of the file the palette has been read from
     */
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    /**
     * \brief Whether it has been modified and it might be advisable to save it
     */
    Q_PROPERTY(bool dirty READ dirty WRITE setDirty NOTIFY dirtyChanged)

public:
    typedef QPair<QColor,QString> value_type;

    ColorPalette(const QVector<QColor>& colors, const QString& name = QString(), int columns = 0);
    ColorPalette(const QVector<QPair<QColor,QString> >& colors, const QString& name = QString(), int columns = 0);
    explicit ColorPalette(const QString& name = QString());
    ColorPalette(const ColorPalette& other);
    ColorPalette& operator=(const ColorPalette& other);
    ~ColorPalette();
    ColorPalette(ColorPalette&& other);
    ColorPalette& operator=(ColorPalette&& other);

    /**
     * \brief Color at the given index
     */
    Q_INVOKABLE QColor colorAt(int index) const;

    /**
     * \brief Color name at the given index
     */
    Q_INVOKABLE QString nameAt(int index) const;

    QVector<QPair<QColor,QString> > colors() const;
    QVector<QColor> onlyColors() const;

    int count() const;
    int columns();

    QString name() const;

    /**
     * \brief Use a color table to set the colors
     */
    Q_INVOKABLE void loadColorTable(const QVector<QRgb>& color_table);

    /**
     * \brief Convert to a color table
     */
    Q_INVOKABLE QVector<QRgb> colorTable() const;
    
    /**
     * \brief Creates a ColorPalette from a color table
     */
    static ColorPalette fromColorTable(const QVector<QRgb>& table);

    /**
     * \brief Use the pixels on an image to set the palette colors
     */
    Q_INVOKABLE bool loadImage(const QImage& image);

    /**
     * \brief Creates a ColorPalette from a Gimp palette (gpl) file
     */
    static ColorPalette fromImage(const QImage& image);

    /**
     * \brief Load contents from a Gimp palette (gpl) file
     * \returns \b true On Success
     * \note If this function returns \b false, the palette will become empty
     */
    Q_INVOKABLE bool load(const QString& name);

    /**
     * \brief Creates a ColorPalette from a Gimp palette (gpl) file
     */
    static ColorPalette fromFile(const QString& name);

    QString fileName() const;

    bool dirty() const;

    /**
     * \brief Returns a preview image of the colors in the palette
     */
    QPixmap preview(const QSize& size, const QColor& background=Qt::transparent) const;

public Q_SLOTS:
    void setColumns(int columns);

    void setColors(const QVector<QColor>& colors);
    void setColors(const QVector<QPair<QColor,QString> >& colors);

    /**
     * \brief Change the color at the given index
     */
    void setColorAt(int index, const QColor& color);
    /**
     * \brief Change the color at the given index
     */
    void setColorAt(int index, const QColor& color, const QString& name);
    /**
     * \brief Change the name of a color
     */
    void setNameAt(int index, const QString& name = QString());
    /**
     * \brief Append a color at the end
     */
    void appendColor(const QColor& color, const QString& name = QString());
    /**
     * \brief Insert a color in an arbitrary location
     */
    void insertColor(int index, const QColor& color, const QString& name = QString());
    /**
     * \brief Remove the color at the given index
     */
    void eraseColor(int index);

    /**
     * \brief Change file name and save
     * \returns \b true on success
     */
    bool save(const QString& filename);
    /**
     * \brief save to file, the filename is \c fileName or determined automatically
     * \returns \b true on success
     */
    bool save();

    void setName(const QString& name);
    void setFileName(const QString& name);
    void setDirty(bool dirty);

Q_SIGNALS:
    /**
     * \brief Emitted when all the colors have changed
     */
    void colorsChanged(const QVector<QPair<QColor,QString> >&);
    void columnsChanged(int);
    void nameChanged(const QString&);
    void fileNameChanged(const QString&);
    void dirtyChanged(bool);
    /**
     * \brief Emitted when the color or the name at the given index has been modified
     */
    void colorChanged(int index);
    /**
     * \brief Emitted when the color at the given index has been removed
     */
    void colorRemoved(int index);
    /**
     * \brief Emitted when a single color has been added
     */
    void colorAdded(int index);
    /**
     * \brief Emitted when the colors have been modified with a simple operation (set, append etc.)
     */
    void colorsUpdated(const QVector<QPair<QColor,QString>>&);

private:
    /**
     * \brief Returns \c name if it isn't null, otherwise a default value
     */
    QString unnamed(const QString& name = QString()) const;

    /**
     * \brief Emit all the necessary signals when the palette has been completely overwritten
     */
    void emitUpdate();

    class Private;
    Private *p;
};

} // namespace color_widgets

#endif // COLOR_WIDGETS_COLOR_PALETTE_HPP
