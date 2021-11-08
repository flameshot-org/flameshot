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
#include "QtColorWidgets/color_palette.hpp"
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QPainter>
#include <QFileInfo>

namespace color_widgets {

class ColorPalette::Private
{
public:
    QVector<QPair<QColor,QString> >   colors;
    int             columns;
    QString         name;
    QString         fileName;
    bool            dirty;

    bool valid_index(int index)
    {
        return index >= 0 && index < colors.size();
    }
};

ColorPalette::ColorPalette(const QVector<QColor>& colors,
                           const QString& name,
                           int columns)
    : p ( new Private )
{
    setName(name);
    setColumns(columns);
    setColors(colors);
}

ColorPalette::ColorPalette(const QString& name)
    : p ( new Private )
{
    setName(name);
    p->columns = 0;
    p->dirty = false;
}

ColorPalette::ColorPalette(const QVector<QPair<QColor,QString> >& colors,
                           const QString& name,
                           int columns)
{
    setName(name);
    setColumns(columns);
    setColors(colors);
    p->dirty = false;
}

ColorPalette::ColorPalette(const ColorPalette& other)
    : QObject(), p ( new Private(*other.p) )
{
}

ColorPalette& ColorPalette::operator=(const ColorPalette& other)
{
    *p = *other.p;
    emitUpdate();
    return *this;
}

ColorPalette::~ColorPalette()
{
    delete p;
}

ColorPalette::ColorPalette(ColorPalette&& other)
    : QObject(), p ( other.p )
{
    other.p = nullptr;
}
ColorPalette& ColorPalette::operator=(ColorPalette&& other)
{
    std::swap(p, other.p);
    emitUpdate();
    return *this;
}

void ColorPalette::emitUpdate()
{
    Q_EMIT colorsChanged(p->colors);
    Q_EMIT columnsChanged(p->columns);
    Q_EMIT nameChanged(p->name);
    Q_EMIT fileNameChanged(p->fileName);
    Q_EMIT dirtyChanged(p->dirty);
}

QColor ColorPalette::colorAt(int index) const
{
    return p->valid_index(index) ? p->colors[index].first : QColor();
}

QString ColorPalette::nameAt(int index) const
{
    return p->valid_index(index) ? p->colors[index].second : QString();
}

QVector<QPair<QColor,QString> > ColorPalette::colors() const
{
    return p->colors;
}

int ColorPalette::count() const
{
    return p->colors.size();
}

int ColorPalette::columns()
{
    return p->columns;
}

QString ColorPalette::name() const
{
    return p->name;
}

void ColorPalette::loadColorTable(const QVector<QRgb>& color_table)
{
    p->colors.clear();
    p->colors.reserve(color_table.size());
    for ( QRgb c : color_table )
    {
        QColor color ( c );
        color.setAlpha(255);
        p->colors.push_back(qMakePair(color,QString()));
    }
    Q_EMIT colorsChanged(p->colors);
    setDirty(true);
}

bool ColorPalette::loadImage(const QImage& image)
{
    if ( image.isNull() )
        return false;
    setColumns(image.width());

    p->colors.clear();
    p->colors.reserve(image.width()*image.height());
    for ( int y = 0; y < image.height(); y++ )
    {
        for ( int x = 0; x < image.width(); x++ )
        {
            QColor color ( image.pixel(x, y) );
            color.setAlpha(255);
            p->colors.push_back(qMakePair(color,QString()));
        }
    }
    Q_EMIT colorsChanged(p->colors);
    setDirty(true);
    return true;
}

ColorPalette ColorPalette::fromImage(const QImage& image)
{
    ColorPalette p;
    p.loadImage(image);
    return p;
}

bool ColorPalette::load(const QString& name)
{
    p->fileName = name;
    p->colors.clear();
    p->columns = 0;
    p->dirty = false;
    p->name = QFileInfo(name).baseName();

    QFile file(name);

    if ( !file.open(QFile::ReadOnly|QFile::Text) )
    {
        emitUpdate();
        return false;
    }

    QTextStream stream( &file );

    if ( stream.readLine() != QLatin1String("GIMP Palette") )
    {
        emitUpdate();
        return false;
    }

    QString line;

    // parse properties
    QHash<QString,QString> properties;
    while( !stream.atEnd() )
    {
        line = stream.readLine();
        if ( line.isEmpty() )
            continue;
        if ( line[0] == '#' )
            break;
        int colon = line.indexOf(':');
        if ( colon == -1 )
            break;
        properties[line.left(colon).toLower()] =
            line.right(line.size() - colon - 1).trimmed();
    }
    /// \todo Store extra properties in the palette object
    setName(properties[QStringLiteral("name")]);
    setColumns(properties[QStringLiteral("columns")].toInt());

    // Skip comments
    if ( !stream.atEnd() && line[0] == '#' )
        while( !stream.atEnd() )
        {
            qint64 pos = stream.pos();
            line = stream.readLine();
            if ( !line.isEmpty() && line[0] != '#' )
            {
                stream.seek(pos);
                break;
            }
        }

    while( !stream.atEnd() )
    {
        int r = 0, g = 0, b = 0;
        stream >> r >> g >> b;
        line = stream.readLine().trimmed();
        p->colors.push_back(qMakePair(QColor(r, g, b), line));
    }

    Q_EMIT colorsChanged(p->colors);
    setDirty(false);

    return true;
}

ColorPalette ColorPalette::fromFile(const QString& name)
{
    ColorPalette p;
    p.load(name);
    return p;
}

bool ColorPalette::save(const QString& filename)
{
    setFileName(filename);
    return save();
}

bool ColorPalette::save()
{
    QString filename = p->fileName;
    if ( filename.isEmpty() )
    {
        filename = unnamed(p->name)+".gpl";
    }

    QFile file(filename);
    if ( !file.open(QFile::Text|QFile::WriteOnly) )
        return false;

    QTextStream stream(&file);

    stream << "GIMP Palette\n";
    stream << "Name: " << unnamed(p->name) << '\n';
    if ( p->columns )
        stream << "Columns: " << p->columns << '\n';
    /// \todo Options to add comments
    stream << "#\n";

    for ( int i = 0; i < p->colors.size(); i++ )
    {
        stream << qSetFieldWidth(3) << p->colors[i].first.red() << qSetFieldWidth(0) << ' '
               << qSetFieldWidth(3) << p->colors[i].first.green() << qSetFieldWidth(0) << ' '
               << qSetFieldWidth(3) << p->colors[i].first.blue() << qSetFieldWidth(0) << '\t'
               << unnamed(p->colors[i].second) << '\n';
    }

    if ( !file.error() )
    {
        setDirty(false);
        return true;
    }

    return false;
}


QString ColorPalette::fileName() const
{
    return p->fileName;
}


void ColorPalette::setColumns(int columns)
{
    if ( columns <= 0 )
        columns = 0;

    if ( columns != p->columns )
    {
        setDirty(true);
        Q_EMIT columnsChanged( p->columns = columns );
    }
}

void ColorPalette::setColors(const QVector<QColor>& colors)
{
    p->colors.clear();
    Q_FOREACH(const QColor& col, colors)
        p->colors.push_back(qMakePair(col,QString()));
    setDirty(true);
    Q_EMIT colorsChanged(p->colors);
}

void ColorPalette::setColors(const QVector<QPair<QColor,QString> >& colors)
{
    p->colors = colors;
    setDirty(true);
    Q_EMIT colorsChanged(p->colors);
}


void ColorPalette::setColorAt(int index, const QColor& color)
{
    if ( !p->valid_index(index) )
        return;

    p->colors[index].first = color;

    setDirty(true);
    Q_EMIT colorChanged(index);
    Q_EMIT colorsUpdated(p->colors);
}

void ColorPalette::setColorAt(int index, const QColor& color, const QString& name)
{
    if ( !p->valid_index(index) )
        return;

    p->colors[index].first = color;
    p->colors[index].second = name;
    setDirty(true);
    Q_EMIT colorChanged(index);
    Q_EMIT colorsUpdated(p->colors);
}

void ColorPalette::setNameAt(int index, const QString& name)
{
    if ( !p->valid_index(index) )
        return;

    p->colors[index].second = name;

    setDirty(true);
    Q_EMIT colorChanged(index);
    Q_EMIT colorsUpdated(p->colors);
}


void ColorPalette::appendColor(const QColor& color, const QString& name)
{
    p->colors.push_back(qMakePair(color,name));
    setDirty(true);
    Q_EMIT colorAdded(p->colors.size()-1);
    Q_EMIT colorsUpdated(p->colors);
}

void ColorPalette::insertColor(int index, const QColor& color, const QString& name)
{
    if ( index < 0 || index > p->colors.size() )
        return;

    p->colors.insert(index, qMakePair(color, name));

    setDirty(true);
    Q_EMIT colorAdded(index);
    Q_EMIT colorsUpdated(p->colors);
}

void ColorPalette::eraseColor(int index)
{
    if ( !p->valid_index(index) )
        return;

    p->colors.remove(index);

    setDirty(true);
    Q_EMIT colorRemoved(index);
    Q_EMIT colorsUpdated(p->colors);
}

void ColorPalette::setName(const QString& name)
{
    setDirty(true);
    p->name = name;
}

void ColorPalette::setFileName(const QString& name)
{
    setDirty(true);
    p->fileName = name;
}

QString ColorPalette::unnamed(const QString& name) const
{
    return name.isEmpty() ? tr("Unnamed") : name;
}


QPixmap ColorPalette::preview(const QSize& size, const QColor& background) const
{
    if ( !size.isValid() || p->colors.empty() )
        return QPixmap();

    QPixmap out( size );
    out.fill(background);
    QPainter painter(&out);

    int count = p->colors.size();
    int columns = p->columns;
    if ( !columns )
        columns = std::ceil( std::sqrt( count * float(size.width()) / size.height() ) );
    int rows = std::ceil( float(count) / columns );
    QSizeF color_size(float(size.width()) / columns, float(size.height()) / rows);

    for ( int y = 0, i = 0; y < rows && i < count; y++ )
    {
        for ( int x = 0; x < columns && i < count; x++, i++ )
        {
            painter.fillRect(QRectF(x*color_size.width(), y*color_size.height(),
                             color_size.width(), color_size.height()),
                             p->colors[i].first
                            );
        }
    }

    return out;
}

bool ColorPalette::dirty() const
{
    return p->dirty;
}

void ColorPalette::setDirty(bool dirty)
{
    if ( dirty != p->dirty )
        Q_EMIT dirtyChanged( p->dirty = dirty );
}

QVector<QColor> ColorPalette::onlyColors() const
{
    QVector<QColor> out;
    out.reserve(p->colors.size());
    for ( int i = 0; i < p->colors.size(); i++ )
        out.push_back(p->colors[i].first);
    return out;
}

QVector<QRgb> ColorPalette::colorTable() const
{
    QVector<QRgb> out;
    out.reserve(p->colors.size());
    for ( const auto& color_pair : p->colors )
        out.push_back(color_pair.first.rgba());
    return out;
}

ColorPalette ColorPalette::fromColorTable(const QVector<QRgb>& table)
{
    ColorPalette palette;
    palette.loadColorTable(table);
    return palette;
}

} // namespace color_widgets
