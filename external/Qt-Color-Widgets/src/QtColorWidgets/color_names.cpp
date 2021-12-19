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
#include "QtColorWidgets/color_names.hpp"
#include <QRegularExpression>

static QRegularExpression regex_qcolor (QStringLiteral("^(?:(?:#[[:xdigit:]]{3})|(?:#[[:xdigit:]]{6})|(?:[[:alpha:]]+))$"));
static QRegularExpression regex_func_rgb (QStringLiteral(R"(^rgb\s*\(\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)$)"));
static QRegularExpression regex_hex_rgba (QStringLiteral("^#[[:xdigit:]]{8}$"));
static QRegularExpression regex_func_rgba (QStringLiteral(R"(^rgba?\s*\(\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\)$)"));

namespace color_widgets {


QString stringFromColor(const QColor& color, bool alpha)
{
    if ( !alpha || color.alpha() == 255 )
        return color.name();
    return color.name()+QStringLiteral("%1").arg(color.alpha(), 2, 16, QChar('0'));
}

QColor colorFromString(const QString& string, bool alpha)
{
    QString xs = string.trimmed();
    QRegularExpressionMatch match;

    match = regex_qcolor.match(xs);
    if ( match.hasMatch() )
    {
        return QColor(xs);
    }

    match = regex_func_rgb.match(xs);
    if ( match.hasMatch() )
    {
        return QColor(
            match.captured(1).toInt(),
            match.captured(2).toInt(),
            match.captured(3).toInt()
        );
    }

    if ( alpha )
    {
        match = regex_hex_rgba.match(xs);
        if ( match.hasMatch() )
        {
            return QColor(
                xs.mid(1,2).toInt(nullptr,16),
                xs.mid(3,2).toInt(nullptr,16),
                xs.mid(5,2).toInt(nullptr,16),
                xs.mid(7,2).toInt(nullptr,16)
            );
        }

        match = regex_func_rgba.match(xs);
        if ( match.hasMatch() )
        {
            return QColor(
                match.captured(1).toInt(),
                match.captured(2).toInt(),
                match.captured(3).toInt(),
                match.captured(4).toInt()
            );
        }
    }

    return QColor();
}

} // namespace color_widgets
