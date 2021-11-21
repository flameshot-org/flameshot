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
#ifndef COLOR_WIDGETS_COLOR_PALETTE_MODEL_HPP
#define COLOR_WIDGETS_COLOR_PALETTE_MODEL_HPP

#include <QAbstractListModel>
#include "color_palette.hpp"

namespace color_widgets {

class QCP_EXPORT ColorPaletteModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * \brief List of directories to be scanned for palette files
     */
    Q_PROPERTY(QStringList searchPaths READ searchPaths WRITE setSearchPaths NOTIFY searchPathsChanged)

    /**
     * \brief Default directory to be used when saving a palette
     */
    Q_PROPERTY(QString savePath READ savePath WRITE setSavePath NOTIFY savePathChanged)

    /**
     * \brief Size of the icon used for the palette previews
     */
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)


public:
    ColorPaletteModel();
    ~ColorPaletteModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;

    QString savePath() const;
    QStringList searchPaths() const;
    QSize iconSize() const;

    /**
     * \brief Number of palettes
     */
    int count() const;

    /**
     * \brief Returns a reference to the first palette with the given name
     * \pre hasPalette(name)
     */
    const ColorPalette& palette(const QString& name) const;

    /**
     * \brief Whether a palette with the given name exists in the model
     */
    bool hasPalette(const QString& name) const;

    /**
     * \brief Get the palette at the given index (row)
     * \pre 0 <= index < count()
     */
    const ColorPalette& palette(int index) const;

    /**
     * \brief Updates an existing palette
     * \param index Palette index
     * \param palette New palette
     * \param save Whether to save the palette to the filesystem
     *
     * Saving will try: (in this order)
     *      * To overwrite the file pointed by the old palette
     *      * To write to the new palette file name
     *      * To create a file in the save path
     * If all of the above fail, the palette will be replaced interally
     * but not on the filesystem
     *
     * \returns \b true if the palette has been successfully updated (and saved)
     */
    bool updatePalette(int index, const ColorPalette& palette, bool save = true);

    /**
     * \brief Remove a palette from the model and optionally from the filesystem
     * \returns \b true if the palette has been successfully removed
     */
    bool removePalette(int index, bool remove_file = true);

    /**
     * \brief Remove a palette to the model and optionally to the filesystem
     * \returns \b true if the palette has been successfully added
     */
    bool addPalette(const ColorPalette& palette, bool save = true);

    /**
     * \brief The index of the palette with the given file name
     * \returns -1 if none is found
     */
    int indexFromFile(const QString& filename) const;

public Q_SLOTS:
    void setSavePath(const QString& savePath);
    void setSearchPaths(const QStringList& searchPaths);
    void addSearchPath(const QString& path);
    void setIconSize(const QSize& iconSize);

    /**
     * \brief Load palettes files found in the search paths
     */
    void load();

Q_SIGNALS:
    void savePathChanged(const QString& savePath);
    void searchPathsChanged(const QStringList& searchPaths);
    void iconSizeChanged(const QSize& iconSize);

private:
    class Private;
    Private* p;
};

} // namespace color_widgets

#endif // COLOR_WIDGETS_COLOR_PALETTE_MODEL_HPP
