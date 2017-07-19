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

#include "filenamehandler.h"
#include "src/utils/confighandler.h"
#include <ctime>
#include <locale>
#include <QStandardPaths>
#include <QDir>

FileNameHandler::FileNameHandler(QObject *parent) : QObject(parent) {
    std::locale::global(std::locale(std::locale("").name()));
}

QString FileNameHandler::getParsedPattern() {
    return parseFilename(ConfigHandler().getFilenamePattern());
}

QString FileNameHandler::parseFilename(const QString &name) {
    QString res;
    if (name.isEmpty()) {
        res = tr("screenshot");
    } else {
        std::time_t t = std::time(NULL);

        char *tempData = QStringTocharArr(name);
        char data[MAX_CHARACTERS] = {0};
        std::strftime(data, sizeof(data),
                      tempData, std::localtime(&t));
        res = QString::fromLocal8Bit(data, strlen(data));
        free(tempData);
    }
    return res;
}

void FileNameHandler::savePattern(const QString &pattern) {
    ConfigHandler().setFilenamePattern(pattern);
}

QString FileNameHandler::getAbsoluteSavePath() {
    ConfigHandler config;
    QString savePath = config.getSavePath();
    bool changed = false;
    if (savePath.isEmpty() || !QDir(savePath).exists() || !QFileInfo(savePath).isWritable()) {
        changed = true;
        savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (savePath.isEmpty()) {
            savePath = QDir::currentPath();
        }
    }
    if(changed) {
        config.setSavePath(savePath);
    }
    QString tempName = "/"+ FileNameHandler().getParsedPattern();
    // find unused name adding _n where n is a number
    QFileInfo checkFile(savePath + tempName + ".png");
    if (checkFile.exists()) {
        tempName += "_";
        int i = 1;
        while (true) {
            checkFile.setFile(
                        savePath + tempName + QString::number(i) + ".png");
            if (!checkFile.exists()) {
                tempName += QString::number(i);
                break;
            }
            ++i;
        }
    }
    return savePath + tempName + ".png";
}

QString FileNameHandler::charArrToQString(const char *c) {
    return QString::fromLocal8Bit(c, MAX_CHARACTERS);
}

char * FileNameHandler::QStringTocharArr(const QString &s) {
    QByteArray ba = s.toLocal8Bit();
    return const_cast<char *>(strdup(ba.constData()));
}
