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

#include <QObject>

class FileNameHandler : public QObject
{
    Q_OBJECT
public:
    explicit FileNameHandler(QObject *parent = nullptr);

    QString parsedPattern();
    QString parseFilename(const QString &name);
    QString generateAbsolutePath(const QString &path);
    QString absoluteSavePath(QString &directory, QString &filename);
    QString absoluteSavePath();


    static const int MAX_CHARACTERS = 70;

public slots:
    void setPattern(const QString &pattern);

private:
    //using charArr = char[MAX_CHARACTERS];
    QString charArrToQString(const char *c);
    char * QStringTocharArr(const QString &s);

    void fixPath(QString &directory, QString &filename);

};
