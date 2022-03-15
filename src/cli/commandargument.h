// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QString>

class CommandArgument
{
public:
    CommandArgument();
    explicit CommandArgument(QString name, QString description);

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
