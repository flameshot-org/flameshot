// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QStringList>
#include <functional>

using std::function;

class CommandOption
{
public:
    CommandOption(const QString& name,
                  QString description,
                  QString valueName = QString(),
                  QString defaultValue = QString());

    CommandOption(QStringList names,
                  QString description,
                  QString valueName = QString(),
                  QString defaultValue = QString());

    void setName(const QString& name);
    void setNames(const QStringList& names);
    QStringList names() const;
    QStringList dashedNames() const;

    void setValueName(const QString& name);
    QString valueName() const;

    void setValue(const QString& value);
    QString value() const;

    void addChecker(const function<bool(QString const&)> checker,
                    const QString& errMsg);
    bool checkValue(const QString& value) const;

    QString description() const;
    void setDescription(const QString& description);

    QString errorMsg() const;

    bool operator==(const CommandOption& option) const;

private:
    QStringList m_names;
    QString m_description;
    QString m_valueName;
    QString m_value;

    function<bool(QString const&)> m_checker;
    QString m_errorMsg;
};
