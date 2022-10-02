// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "winlnkfileparse.h"
#include <QDir>
#include <QDirIterator>
#include <QFileSystemModel>
#include <QImageWriter>
#include <QRegularExpression>
#include <QSettings>
#include <QString>

#include <shlobj.h>

WinLnkFileParser::WinLnkFileParser()
{
    QStringList sListImgFileExt;
    for (const auto& ext : QImageWriter::supportedImageFormats()) {
        sListImgFileExt.append(ext);
    }
    this->getImageFileExtAssociates(sListImgFileExt);
}

DesktopAppData WinLnkFileParser::parseLnkFile(const QFileInfo& fiLnk,
                                              bool& ok) const
{
    DesktopAppData res;
    ok = true;

    QFileInfo fiSymlink(fiLnk.symLinkTarget());
    if (!fiSymlink.exists() || !fiSymlink.fileName().endsWith(".exe") ||
        fiSymlink.baseName().contains("unins")) {
        ok = false;
        return res;
    }

    res.name = fiLnk.baseName();
    res.exec = fiSymlink.absoluteFilePath();

    // Get icon from exe
    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath(fiSymlink.path());
    res.icon = model->fileIcon(model->index(fiSymlink.filePath()));

    if (m_GraphicAppsList.contains(fiSymlink.fileName())) {
        res.categories = QStringList() << "Graphics";
    } else {
        res.categories = QStringList() << "Utility";
    }

    for (const auto& app : m_appList) {
        if (app.exec == res.exec) {
            ok = false;
            break;
        }
    }

    if (res.exec.isEmpty() || res.name.isEmpty()) {
        ok = false;
    }
    return res;
}

int WinLnkFileParser::processDirectory(const QDir& dir)
{
    QStringList sListMenuFilter;
    sListMenuFilter << "Accessibility"
                    << "Administrative Tools"
                    << "Setup"
                    << "System Tools"
                    << "Uninstall"
                    << "Update"
                    << "Updater"
                    << "Windows PowerShell";
    const QString sMenuFilter("\\b(" + sListMenuFilter.join('|') + ")\\b");
    QRegularExpression regexfilter(sMenuFilter);

    bool ok;
    int length = m_appList.length();
    // Go through all subfolders and *.lnk files
    QDirIterator it(dir.absolutePath(),
                    { "*.lnk" },
                    QDir::NoFilter,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo fiLnk(it.next());
        if (!regexfilter.match(fiLnk.absoluteFilePath()).hasMatch()) {
            DesktopAppData app = parseLnkFile(fiLnk, ok);
            if (ok) {
                m_appList.append(app);
            }
        }
    }

    return m_appList.length() - length;
}

QVector<DesktopAppData> WinLnkFileParser::getAppsByCategory(
  const QString& category)
{
    QVector<DesktopAppData> res;
    for (const DesktopAppData& app : qAsConst(m_appList)) {
        if (app.categories.contains(category)) {
            res.append(app);
        }
    }

    std::sort(res.begin(), res.end(), CompareAppByName());

    return res;
}

QMap<QString, QVector<DesktopAppData>> WinLnkFileParser::getAppsByCategory(
  const QStringList& categories)
{
    QMap<QString, QVector<DesktopAppData>> res;

    QVector<DesktopAppData> tmpAppList;
    for (const QString& category : categories) {
        tmpAppList = getAppsByCategory(category);
        for (const DesktopAppData& app : qAsConst(tmpAppList)) {
            res[category].append(app);
        }
    }

    return res;
}

QString WinLnkFileParser::getAllUsersStartMenuPath()
{
    QString sRet("");
    WCHAR path[MAX_PATH];
    HRESULT hr = SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, path);

    if (SUCCEEDED(hr)) {
        sRet = QDir(QString::fromWCharArray(path)).absolutePath();
    }

    return sRet;
}

void WinLnkFileParser::getImageFileExtAssociates(const QStringList& sListImgExt)
{
    const QString sReg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\"
                       "CurrentVersion\\Explorer\\FileExts\\.%1\\OpenWithList");

    for (const auto& sExt : qAsConst(sListImgExt)) {
        QString sPath(sReg.arg(sExt));
        QSettings registry(sPath, QSettings::NativeFormat);
        for (const auto& key : registry.allKeys()) {
            if (1 == key.size()) { // Keys for OpenWith apps are a, b, c, ...
                QString sVal = registry.value(key, "").toString();
                if (sVal.endsWith(".exe") &&
                    !m_GraphicAppsList.contains(sVal)) {
                    m_GraphicAppsList << sVal;
                }
            }
        }
    }
}
