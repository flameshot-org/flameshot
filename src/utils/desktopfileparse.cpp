// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "desktopfileparse.h"
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QLocale>

DesktopFileParser::DesktopFileParser() {
    QString locale = QLocale().name();
    QString localeShort = QLocale().name().left(2);
    m_localeName = QStringLiteral("Name[%1]").arg(locale);
    m_localeDescription = QStringLiteral("Comment[%1]").arg(locale);
    m_localeNameShort = QStringLiteral("Name[%1]").arg(localeShort);
    m_localeDescriptionShort = QStringLiteral("Comment[%1]")
            .arg(localeShort);
    m_defaultIcon = QIcon::fromTheme(QStringLiteral("application-x-executable"));
}

DesktopAppData DesktopFileParser::parseDesktopFile(
        const QString &fileName, bool &ok) const
{
    DesktopAppData res;
    ok = true;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ok = false;
        return res;
    }
    bool nameLocaleSet = false;
    bool descriptionLocaleSet = false;
    bool isApplication = false;
    QTextStream in(&file);
    // enter the desktop entry definition
    while (!in.atEnd() && in.readLine() != QLatin1String("[Desktop Entry]")) {
    }
    // start parsing
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith(QLatin1String("Icon"))) {
            res.icon = QIcon::fromTheme(
                      line.mid(line.indexOf(QLatin1String("="))+1).trimmed(),
                        m_defaultIcon);
        }
        else if (!nameLocaleSet && line.startsWith(QLatin1String("Name"))) {
            if (line.startsWith(m_localeName) ||
                    line.startsWith(m_localeNameShort))
            {
                res.name = line.mid(line.indexOf(QLatin1String("="))+1).trimmed();
                nameLocaleSet = true;
            } else if (line.startsWith(QLatin1String("Name="))) {
                res.name = line.mid(line.indexOf(QLatin1String("="))+1).trimmed();
            }
        }
        else if (!descriptionLocaleSet && line.startsWith(QLatin1String("Comment"))) {
            if (line.startsWith(m_localeDescription) ||
                    line.startsWith(m_localeDescriptionShort))
            {
                res.description = line.mid(line.indexOf(QLatin1String("="))+1).trimmed();
                descriptionLocaleSet = true;
            } else if (line.startsWith(QLatin1String("Comment="))) {
                res.description = line.mid(line.indexOf(QLatin1String("="))+1).trimmed();
            }
        }
        else if (line.startsWith(QLatin1String("Exec"))) {
            if (line.contains(QLatin1String("%"))) {
                res.exec = line.mid(line.indexOf(QLatin1String("="))+1)
                        .trimmed();
            } else {
                ok = false;
                break;
            }
        }
        else if (line.startsWith(QLatin1String("Type"))) {
            if (line.contains(QLatin1String("Application"))) {
                isApplication = true;
            }
        }
        else if (line.startsWith(QLatin1String("Categories"))) {
            res.categories = line.mid(line.indexOf(QLatin1String("="))+1).split(QStringLiteral(";"));
        }
        else if (line == QLatin1String("NoDisplay=true")) {
            ok = false;
            break;
        }
        else if (line == QLatin1String("Terminal=true")) {
            res.showInTerminal = true;
        }
        // ignore the other entries
        else if (line.startsWith(QLatin1String("["))) {
            break;
        }
    }
    file.close();
    if (res.exec.isEmpty() || res.name.isEmpty() || !isApplication) {
        ok = false;
    }
    return res;
}

int DesktopFileParser::processDirectory(const QDir &dir) {
    QStringList entries = dir.entryList(QDir::NoDotAndDotDot | QDir::Files);
    bool ok;
    int length = m_appList.length();
    for (QString file: entries){
        DesktopAppData app = parseDesktopFile(dir.absoluteFilePath(file), ok);
        if (ok) {
            m_appList.append(app);
        }
    }
    return m_appList.length() - length;
}

QVector<DesktopAppData> DesktopFileParser::getAppsByCategory(const QString &category) {
    QVector<DesktopAppData> res;
    for (const DesktopAppData &app : m_appList) {
        if (app.categories.contains(category)) {
            res.append(app);
        }
    }
    return res;
}

QMap<QString, QVector<DesktopAppData>> DesktopFileParser::getAppsByCategory(
        const QStringList &categories)
{
    QMap<QString, QVector<DesktopAppData>> res;
    for (const DesktopAppData &app : m_appList) {
        for (const QString &category: categories) {
            if (app.categories.contains(category)) {
                res[category].append(app);
            }
        }
    }
    return res;
}
