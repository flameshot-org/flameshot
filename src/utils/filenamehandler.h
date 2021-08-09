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

    QString properScreenshotPath(QString filename,
                                 const QString& format = QString());

    static const int MAX_CHARACTERS = 70;

private:
    QString autoNumerateDuplicate(QString path);
};
