// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QObject>

class FileNameHandler : public QObject
{
    Q_OBJECT
public:
    explicit FileNameHandler(QObject* parent = nullptr);

    QString parsedPattern();
    QString parseFilename(const QString& name);
    QString generateAbsolutePath(const QString& path);
    QString absoluteSavePath(QString& directory, QString& filename);
    QString absoluteSavePath();

    static const int MAX_CHARACTERS = 70;

public slots:
    void setPattern(const QString& pattern);

private:
    // using charArr = char[MAX_CHARACTERS];
    QString charArrToQString(const char* c);
    char* QStringToCharArr(const QString& s);

    void fixPath(QString& directory, QString& filename);
};
