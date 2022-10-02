// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "desktopfileparse.h"
#include <QFileInfo>
#include <QIcon>
#include <QMap>
#include <QStringList>

class QDir;
class QString;

struct CompareAppByName
{
    bool operator()(const DesktopAppData a, const DesktopAppData b)
    {
        return (a.name < b.name);
    }
};

struct WinLnkFileParser
{
    WinLnkFileParser();
    DesktopAppData parseLnkFile(const QFileInfo& fiLnk, bool& ok) const;
    int processDirectory(const QDir& dir);
    QString getAllUsersStartMenuPath();

    QVector<DesktopAppData> getAppsByCategory(const QString& category);
    QMap<QString, QVector<DesktopAppData>> getAppsByCategory(
      const QStringList& categories);

private:
    void getImageFileExtAssociates(const QStringList& sListImgExt);

    QVector<DesktopAppData> m_appList;
    QStringList m_GraphicAppsList;
};
