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

#include "commandargument.h"

CommandArgument::CommandArgument() {

}

CommandArgument::CommandArgument(const QString &name,
                                 const QString &description) :
    m_name(name), m_description(description)
{

}

void CommandArgument::setName(const QString &name) {
    m_name = name;
}

QString CommandArgument::name() const {
    return m_name;
}

void CommandArgument::setDescription(const QString &description) {
    m_description = description;
}

QString CommandArgument::description() const {
    return m_description;
}

bool CommandArgument::isRoot() const {
    return m_name.isEmpty() && m_description.isEmpty();
}

bool CommandArgument::operator ==(const CommandArgument &arg) const {
    return m_description == arg.m_description
            && m_name == arg.m_name;
}
