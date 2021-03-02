// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "filenamehandler.h"
#include "src/utils/confighandler.h"
#include "src/utils/strfparse.h"
#include <QDir>
#include <QStandardPaths>
#include <ctime>
#include <exception>
#include <locale>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"

FileNameHandler::FileNameHandler(QObject* parent)
  : QObject(parent)
{
    try {
        std::locale::global(std::locale(""));
    } catch (std::exception& e) {
        spdlog::error("Locales on your system are not properly configured. "
                      "Falling back to defaults");
        std::locale::global(std::locale("en_US.UTF-8"));
    }
}

QString FileNameHandler::parsedPattern()
{
    return parseFilename(ConfigHandler().filenamePatternValue());
}

QString FileNameHandler::parseFilename(const QString& name)
{
    QString res = name;
    if (name.isEmpty()) {
        res = ConfigHandler().filenamePatternDefault();
    }

    // remove trailing characters '%' in the pattern
    while (res.endsWith('%')) {
        res.chop(1);
    }

    res =
      QString::fromStdString(strfparse::format_time_string(name.toStdString()));

    // add the parsed pattern in a correct format for the filesystem
    res = res.replace(QLatin1String("/"), QStringLiteral("⁄"))
            .replace(QLatin1String(":"), QLatin1String("-"));
    return res;
}

QString FileNameHandler::generateAbsolutePath(const QString& path)
{
    QString directory = path;
    QString filename = parsedPattern();
    fixPath(directory, filename);
    return directory + filename;
}
// path a images si no existe, add numeration
void FileNameHandler::setPattern(const QString& pattern)
{
    ConfigHandler().setFilenamePattern(pattern);
}

QString FileNameHandler::absoluteSavePath(QString& directory, QString& filename)
{
    ConfigHandler config;
    directory = config.savePath();
    if (directory.isEmpty() || !QDir(directory).exists() ||
        !QFileInfo(directory).isWritable()) {
        directory =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    filename = parsedPattern();
    fixPath(directory, filename);
    return directory + filename;
}

QString FileNameHandler::absoluteSavePath()
{
    QString dir, file;
    return absoluteSavePath(dir, file);
}

QString FileNameHandler::charArrToQString(const char* c)
{
    return QString::fromLocal8Bit(c, MAX_CHARACTERS);
}

char* FileNameHandler::QStringToCharArr(const QString& s)
{
    QByteArray ba = s.toLocal8Bit();
    return const_cast<char*>(strdup(ba.constData()));
}

void FileNameHandler::fixPath(QString& directory, QString& filename)
{
    // add '/' at the end of the directory
    if (!directory.endsWith(QLatin1String("/"))) {
        directory += QLatin1String("/");
    }
    // add numeration in case of repeated filename in the directory
    // find unused name adding _n where n is a number
    QFileInfo checkFile(directory + filename + ".png");
    if (checkFile.exists()) {
        filename += QLatin1String("_");
        int i = 1;
        while (true) {
            checkFile.setFile(directory + filename + QString::number(i) +
                              ".png");
            if (!checkFile.exists()) {
                filename += QString::number(i);
                break;
            }
            ++i;
        }
    }
}
