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

#include <QStringList>
#include <functional>

using std::function;

class CommandOption
{
public:
    CommandOption(const QString &name, const QString &description,
                  const QString &valueName = QString(),
                  const QString &defaultValue = QString());

    CommandOption(const QStringList &names, const QString &description,
                  const QString &valueName = QString(),
                  const QString &defaultValue = QString());

    void setName(const QString &name);
    void setNames(const QStringList &names);
    QStringList names() const;

    void setValueName(const QString &name);
    QString valueName() const;

    void setValue(const QString &value);
    QString value() const;

    void addChecker(const function<bool(QString const&)> checker, const QString &errMsg);
    bool checkValue(const QString &value) const;

    QString description() const;
    void setDescription(const QString &description);

    QString errorMsg() const;

    bool operator==(const CommandOption &option) const;

private:
    QStringList m_names;
    QString m_description;
    QString m_valueName;
    QString m_value;

    function<bool(QString const&)> m_checker;
    QString m_errorMsg;

};
