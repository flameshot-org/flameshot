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
#include "QtColorWidgets/color_palette_model.hpp"
#include <QDir>
#include <QList>
#include <QRegularExpression>

namespace color_widgets {

class ColorPaletteModel::Private
{
public:
    /// \todo Keep sorted by name (?)
    QList<ColorPalette> palettes;
    QSize icon_size;
    QStringList search_paths;
    QString     save_path;

    Private()
        : icon_size(32, 32)
    {}

    bool acceptable(const QModelIndex& index) const
    {
        return acceptable(index.row());
    }

    bool acceptable(int row) const
    {
        return row >= 0 && row <= palettes.count();
    }

    QList<ColorPalette>::iterator find(const QString& name)
    {
        return std::find_if(palettes.begin(), palettes.end(),
            [&name](const ColorPalette& palette) {
                return palette.name() == name;
        });
    }

    bool attemptSave(ColorPalette& palette, const QString& filename)
    {
        if ( filename.isEmpty() )
            return false;
        return palette.save(filename);
    }

    void fixUnnamed(ColorPalette& palette)
    {
        if ( palette.name().isEmpty() )
            palette.setName(ColorPaletteModel::tr("Unnamed"));
    }

    bool save(ColorPalette& palette, const QString& suggested_filename = QString())
    {
        // Attempt to save with the existing file names
        if ( !suggested_filename.isEmpty() && attemptSave(palette, suggested_filename) )
            return true;
        if ( attemptSave(palette, palette.fileName()) )
            return true;

        // Set up the save directory
        QDir save_dir(save_path);
        if ( !save_dir.exists() && !QDir().mkdir(save_path) )
            return false;

        // Attempt to save as (Name).gpl
        QString filename = palette.name()+".gpl";
        if ( !save_dir.exists(filename) &&
                attemptSave(palette, save_dir.absoluteFilePath(filename)) )
            return true;

        // Get all of the files matching the pattern *.gpl
        save_dir.setNameFilters(QStringList() << QStringLiteral("*.gpl"));
        save_dir.setFilter(QDir::Files);
        QStringList existing_files = save_dir.entryList();

        // For all the files that match (Name)(Number).gpl, find the maximum (Number)
        QRegularExpression name_regex(QRegularExpression::escape(palette.name())+"([0-9]+)\\.gpl");
        int max = 0;
        for ( const auto& existing_file : existing_files )
        {
            QRegularExpressionMatch match = name_regex.match(existing_file);
            if ( match.hasMatch() )
            {
                int num = match.captured(1).toInt();
                if ( num > max )
                    max = num;
            }
        }

        return attemptSave(palette,
            save_dir.absoluteFilePath(QStringLiteral("%1%2.gpl").arg(palette.name()).arg(max+1))
        );
    }
};

ColorPaletteModel::ColorPaletteModel()
    : p ( new Private )
    {}

ColorPaletteModel::~ColorPaletteModel()
{
    delete p;
}

int ColorPaletteModel::rowCount(const QModelIndex &) const
{
    return count();
}

QVariant ColorPaletteModel::data(const QModelIndex &index, int role) const
{
    if ( !p->acceptable(index) )
        return QVariant();

    const ColorPalette& palette = p->palettes[index.row()];
    switch( role )
    {
        case Qt::DisplayRole:
            return palette.name();
        case Qt::DecorationRole:
            return palette.preview(p->icon_size);
        case Qt::ToolTipRole:
            return tr("%1 (%2 colors)").arg(palette.name()).arg(palette.count());
    }

    return QVariant();
}

bool ColorPaletteModel::removeRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent)

    if ( !p->acceptable(row) || count <= 0 )
        return false;

    auto begin = p->palettes.begin() + row;
    auto end = row + count >= p->palettes.size() ? p->palettes.end() : begin + count;
    for ( auto it = begin; it != end; ++it )
    {
        if ( !it->fileName().isEmpty() )
        {
            QFileInfo file(it->fileName());
            if ( file.isWritable() && file.isFile() )
                QFile::remove(it->fileName());
        }
    }

    p->palettes.erase(begin, end);

    return true;
}

QSize ColorPaletteModel::iconSize() const
{
    return p->icon_size;
}

void ColorPaletteModel::setIconSize(const QSize& iconSize)
{
    if ( p->icon_size != iconSize )
        Q_EMIT iconSizeChanged( p->icon_size = iconSize );
}

QString ColorPaletteModel::savePath() const
{
    return p->save_path;
}

QStringList ColorPaletteModel::searchPaths() const
{
    return p->search_paths;
}

void ColorPaletteModel::setSavePath(const QString& savePath)
{
    if ( p->save_path != savePath )
        Q_EMIT savePathChanged( p->save_path = savePath );
}

void ColorPaletteModel::setSearchPaths(const QStringList& searchPaths)
{
    if ( p->search_paths != searchPaths )
        Q_EMIT searchPathsChanged( p->search_paths = searchPaths );
}

void ColorPaletteModel::addSearchPath(const QString& path)
{
    /// \todo Should compare canonical paths
    /// and these checks should also be made in setSearchPaths
    if ( !p->search_paths.contains(path) )
    {
        p->search_paths.push_back(path);
        Q_EMIT searchPathsChanged( p->search_paths );
    }
}

void ColorPaletteModel::load()
{
    beginResetModel();
    p->palettes.clear();
    QStringList filters;
    filters << QStringLiteral("*.gpl");
    for ( const QString& directory_name : p->search_paths )
    {
        QDir directory(directory_name);
        directory.setNameFilters(filters);
        directory.setFilter(QDir::Files|QDir::Readable);
        directory.setSorting(QDir::Name);
        for ( const QFileInfo& file : directory.entryInfoList() )
        {
            ColorPalette palette;
            if ( palette.load(file.absoluteFilePath()) )
            {
                p->palettes.push_back(palette);
            }
        }
    }
    endResetModel();
}

bool ColorPaletteModel::hasPalette(const QString& name) const
{
    return p->find(name) != p->palettes.end();
}

int ColorPaletteModel::count() const
{
    return p->palettes.size();
}

const ColorPalette& ColorPaletteModel::palette(const QString& name) const
{
    return *p->find(name);
}

const ColorPalette& ColorPaletteModel::palette(int index) const
{
    return p->palettes[index];
}

bool ColorPaletteModel::updatePalette(int index, const ColorPalette& palette, bool save)
{
    if ( !p->acceptable(index) )
        return false;

    // Store the old file name
    QString filename = p->palettes[index].fileName();
    // Update the palette
    ColorPalette& local_palette = p->palettes[index] = palette;
    p->fixUnnamed(local_palette);

    Q_EMIT dataChanged(this->index(index), this->index(index));

    if ( save )
        return p->save(local_palette, filename);

    return true;
}

bool ColorPaletteModel::removePalette(int index, bool remove_file)
{
    if ( !p->acceptable(index) )
        return false;

    QString file_name = p->palettes[index].fileName();

    beginRemoveRows(QModelIndex(), index, index);
    p->palettes.removeAt(index);
    endRemoveRows();

    if ( !file_name.isEmpty() && remove_file )
    {
        QFileInfo file(file_name);
        if ( file.isWritable() && file.isFile() )
            return QFile::remove(file_name);
        return false;
    }

    return true;
}

bool ColorPaletteModel::addPalette(const ColorPalette& palette,  bool save)
{
    beginInsertRows(QModelIndex(), p->palettes.size(), p->palettes.size());
    p->palettes.push_back(palette);
    p->fixUnnamed(p->palettes.back());
    endInsertRows();

    if ( save )
        return p->save(p->palettes.back());

    return true;
}


int ColorPaletteModel::indexFromFile(const QString& filename) const
{
    QString canonical = QFileInfo(filename).canonicalFilePath();
    int i = 0;
    for ( const auto& pal : p->palettes )
    {
        if ( !pal.fileName().isEmpty() &&
                QFileInfo(pal.fileName()).canonicalFilePath() == canonical )
            return i;
        i++;
    }
    return -1;
}

} // namespace color_widgets
