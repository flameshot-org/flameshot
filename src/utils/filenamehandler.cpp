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

#include "filenamehandler.h"
#include "src/utils/confighandler.h"
#include <QDir>
#include <QStandardPaths>
#include <ctime>
#include <locale>

FileNameHandler::FileNameHandler(QObject* parent)
  : QObject(parent)
{
    std::locale::global(std::locale(""));
}

QString FileNameHandler::parsedPattern()
{
    return parseFilename(ConfigHandler().filenamePatternValue());
}

QString FileNameHandler::parseFilename(const QString& name)
{
    QString res = name;
    // remove trailing characters '%' in the pattern
    if (name.isEmpty()) {
        res = QLatin1String("%F_%H-%M");
    }
    while (res.endsWith('%')) {
        res.chop(1);
    }
    std::time_t t = std::time(NULL);

    char* tempData = QStringTocharArr(res);
    char data[MAX_CHARACTERS] = { 0 };
    std::strftime(data, sizeof(data), tempData, std::localtime(&t));
    res = QString::fromLocal8Bit(data, (int)strlen(data));
    free(tempData);

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

char* FileNameHandler::QStringTocharArr(const QString& s)
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
