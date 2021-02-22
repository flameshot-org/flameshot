// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#ifndef FLAMESHOT_UPLOADSTORAGECONFIG_H
#define FLAMESHOT_UPLOADSTORAGECONFIG_H

#include <QObject>
#include <QWidget>

class QVBoxLayout;

class UploadStorageConfig : public QWidget
{
    Q_OBJECT
public:
    explicit UploadStorageConfig(QWidget* parent = nullptr);

private:
    QVBoxLayout* m_layout;
};

#endif // FLAMESHOT_UPLOADSTORAGECONFIG_H
