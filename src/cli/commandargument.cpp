// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "commandargument.h"

#include <utility>

CommandArgument::CommandArgument() = default;

CommandArgument::CommandArgument(QString name, QString description)
  : m_name(std::move(name))
  , m_description(std::move(description))
{}

void CommandArgument::setName(const QString& name)
{
    m_name = name;
}

QString CommandArgument::name() const
{
    return m_name;
}

void CommandArgument::setDescription(const QString& description)
{
    m_description = description;
}

QString CommandArgument::description() const
{
    return m_description;
}

bool CommandArgument::isRoot() const
{
    return m_name.isEmpty() && m_description.isEmpty();
}

bool CommandArgument::operator==(const CommandArgument& arg) const
{
    return m_description == arg.m_description && m_name == arg.m_name;
}
