// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "commandoption.h"

#include <utility>

CommandOption::CommandOption(const QString& name,
                             QString description,
                             QString valueName,
                             QString defaultValue)
  : m_names(name)
  , m_description(std::move(description))
  , m_valueName(std::move(valueName))
  , m_value(std::move(defaultValue))
{
    m_checker = [](QString const&) { return true; };
}

CommandOption::CommandOption(QStringList names,
                             QString description,
                             QString valueName,
                             QString defaultValue)
  : m_names(std::move(names))
  , m_description(std::move(description))
  , m_valueName(std::move(valueName))
  , m_value(std::move(defaultValue))
{
    m_checker = [](QString const&) -> bool { return true; };
}

void CommandOption::setName(const QString& name)
{
    m_names = QStringList() << name;
}

void CommandOption::setNames(const QStringList& names)
{
    m_names = names;
}

QStringList CommandOption::names() const
{
    return m_names;
}

QStringList CommandOption::dashedNames() const
{
    QStringList dashedNames;
    for (const QString& name : m_names) {
        // prepend "-" to single character options, and "--" to the others
        QString dashedName = (name.length() == 1)
                               ? QStringLiteral("-%1").arg(name)
                               : QStringLiteral("--%1").arg(name);
        dashedNames << dashedName;
    }
    return dashedNames;
}

void CommandOption::setValueName(const QString& name)
{
    m_valueName = name;
}

QString CommandOption::valueName() const
{
    return m_valueName;
}

void CommandOption::setValue(const QString& value)
{
    if (m_valueName.isEmpty()) {
        m_valueName = QLatin1String("value");
    }
    m_value = value;
}

QString CommandOption::value() const
{
    return m_value;
}

void CommandOption::addChecker(const function<bool(const QString&)> checker,
                               const QString& errMsg)
{
    m_checker = checker;
    m_errorMsg = errMsg;
}

bool CommandOption::checkValue(const QString& value) const
{
    return m_checker(value);
}

QString CommandOption::description() const
{
    return m_description;
}

void CommandOption::setDescription(const QString& description)
{
    m_description = description;
}

QString CommandOption::errorMsg() const
{
    return m_errorMsg;
}

bool CommandOption::operator==(const CommandOption& option) const
{
    return m_description == option.m_description && m_names == option.m_names &&
           m_valueName == option.m_valueName;
}
