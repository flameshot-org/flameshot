#include "commandargument.h"
#include <utility>

/**
 * @class CommandArgument
 * @brief A class that represents a command line argument with a name and description.
 */

class CommandArgument {
public:
    CommandArgument();
    CommandArgument(QString name, QString description);

    void setName(const QString& name);
    QString name() const;

    void setDescription(const QString& description);
    QString description() const;

    bool isRoot() const;
    bool operator==(const CommandArgument& arg) const;

private:
    QString m_name;
    QString m_description;
};

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
