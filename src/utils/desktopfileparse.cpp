// Copyright 2017 Alejandro Sirgo Rica
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
#include <QString>
#include <QTextStream>
#include <QLocale>

DesktopFileParse::DesktopFileParse() {
    QString locale = QLocale().name();
    QString localeShort = QLocale().name().left(2);
    m_localeName = QString("Name[%1]").arg(locale);
    m_localeDescription = QString("Comment[%1]").arg(locale);
    m_localeNameShort = QString("Name[%1]").arg(localeShort);
    m_localeDescriptionShort = QString("Comment[%1]")
            .arg(localeShort);
}

DesktopAppData DesktopFileParse::parseDesktopFile(
        const QString &fileName, bool &ok)
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
    bool isGraphics = false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("Icon")) {
            res.icon = QIcon::fromTheme(
                      line.mid(line.indexOf("=")+1).trimmed());
        }
        else if (!nameLocaleSet && line.startsWith("Name")) {
            if (line.startsWith(m_localeName) ||
                    line.startsWith(m_localeNameShort))
            {
                res.name = line.mid(line.indexOf("=")+1).trimmed();
                nameLocaleSet = true;
            } else if (line.startsWith("Name=")) {
                res.name = line.mid(line.indexOf("=")+1).trimmed();
            }
        }
        else if (!descriptionLocaleSet && line.startsWith("Comment")) {
            if (line.startsWith(m_localeDescription) ||
                    line.startsWith(m_localeDescriptionShort))
            {
                res.description = line.mid(line.indexOf("=")+1).trimmed();
                descriptionLocaleSet = true;
            } else if (line.startsWith("Comment=")) {
                res.description = line.mid(line.indexOf("=")+1).trimmed();
            }
        }
        else if (line.startsWith("MimeType") &&
                   !line.contains("image/png"))
        {
            ok = false;
            break;
        }
        else if (line.startsWith("Exec")) {
            if (line.contains("%")) {
                res.exec = line.mid(line.indexOf("=")+1)
                        .trimmed();
            } else {
                ok = false;
                break;
            }
        }
        else if (line.startsWith("Type")) {
            if (line.contains("Application")) {
                isApplication = true;
            }
        }
        else if (line.startsWith("Categories")) {
            if (line.contains("Graphics")) {
                isGraphics = true;
            }
        }
    }
    file.close();
    if (res.exec.isEmpty() || res.name.isEmpty() || !isApplication ||
            !isGraphics) {
        ok = false;
    }
    return res;
}
