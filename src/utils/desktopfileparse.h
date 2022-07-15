// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QIcon>
#include <QMap>
#include <QStringList>

class QDir;
class QString;
class QTextStream;

struct DesktopAppData
{
    DesktopAppData()
      : showInTerminal()
    {}

    DesktopAppData(const QString& name,
                   const QString& description,
                   const QString& exec,
                   QIcon icon)
      : name(name)
      , description(description)
      , exec(exec)
      , icon(icon)
      , showInTerminal(false)
    {}

    bool operator==(const DesktopAppData& other) const
    {
        return name == other.name;
    }

    QString name;
    QString description;
    QString exec;
    QStringList categories;
    QIcon icon;
    bool showInTerminal;
};

struct DesktopFileParser
{
    DesktopFileParser();
    DesktopAppData parseDesktopFile(const QString& fileName, bool& ok) const;
    int processDirectory(const QDir& dir);

    QVector<DesktopAppData> getAppsByCategory(const QString& category);
    QMap<QString, QVector<DesktopAppData>> getAppsByCategory(
      const QStringList& categories);

private:
    QString m_localeName;
    QString m_localeDescription;
    QString m_localeNameShort;
    QString m_localeDescriptionShort;

    QIcon m_defaultIcon;
    QVector<DesktopAppData> m_appList;
};
