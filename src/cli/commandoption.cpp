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

#include "commandoption.h"

CommandOption::CommandOption(const QString &name, const QString &description,
                             const QString &valueName,
                             const QString &defaultValue) :
    m_names(name), m_description(description), m_valueName(valueName),
    m_value(defaultValue)
{
    m_checker = [](QString const&){ return true; };
}

CommandOption::CommandOption(const QStringList &names,
                             const QString &description,
                             const QString &valueName,
                             const QString &defaultValue) :
    m_names(names), m_description(description), m_valueName(valueName),
    m_value(defaultValue)
{
    m_checker = [](QString const&) -> bool { return true; };
}

void CommandOption::setName(const QString &name) {
    m_names = QStringList() << name;
}

void CommandOption::setNames(const QStringList &names) {
    m_names = names;
}

QStringList CommandOption::names() const {
    return m_names;
}

void CommandOption::setValueName(const QString &name) {
    m_valueName = name;
}

QString CommandOption::valueName() const {
    return m_valueName;
}

void CommandOption::setValue(const QString &value) {
    if (m_valueName.isEmpty()) {
        m_valueName = "value";
    }
    m_value = value;
}

QString CommandOption::value() const {
    return m_value;
}

void CommandOption::addChecker(const function<bool (const QString &)> checker,
                               const QString &errMsg)
{
    m_checker = checker;
    m_errorMsg = errMsg;
}

bool CommandOption::checkValue(const QString &value) const {
    return m_checker(value);
}

QString CommandOption::description() const
{
    return m_description;
}

void CommandOption::setDescription(const QString &description)
{
    m_description = description;
}

QString CommandOption::errorMsg() const {
    return m_errorMsg;
}

bool CommandOption::operator ==(const CommandOption &option) const
{
    return m_description == option.m_description
            && m_names == option.m_names
            && m_valueName == option.m_valueName;
}
