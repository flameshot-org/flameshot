// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QString>

class CommandArgument
{
public:
    CommandArgument();
    explicit CommandArgument(const QString &name, const QString &description);

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    bool isRoot() const;

    bool operator ==(const CommandArgument &arg) const;

private:
    QString m_name;
    QString m_description;

};
