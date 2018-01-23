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

#include <QIcon>
#include <QStringList>
#include <QMap>

class QDir;
class QString;
class QTextStream;

struct DesktopAppData {
    DesktopAppData() : showInTerminal() {}

    DesktopAppData(
            QString name,
            QString description,
            QString exec,
            QIcon icon) :
        name(name),
        description(description),
        exec(exec),
        icon(icon),
        showInTerminal(false)
    {}

    bool operator==(const DesktopAppData &other) const {
        return name == other.name;
    }

    QString name;
    QString description;
    QString exec;
    QStringList categories;
    QIcon icon;
    bool showInTerminal;
};

struct DesktopFileParser {
    DesktopFileParser();
    DesktopAppData parseDesktopFile(const QString &fileName, bool &ok) const;
    int processDirectory(const QDir &dir);

    QVector<DesktopAppData> getAppsByCategory(const QString &category);
    QMap<QString, QVector<DesktopAppData>> getAppsByCategory(
            const QStringList &categories);

private:
    QString m_localeName;
    QString m_localeDescription;
    QString m_localeNameShort;
    QString m_localeDescriptionShort;

    QIcon m_defaultIcon;
    QVector<DesktopAppData> m_appList;
};
