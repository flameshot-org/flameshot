// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/storage/imguploadertool.h"

class ImgurUploaderTool : public ImgUploaderTool
{
    Q_OBJECT
public:
    explicit ImgurUploaderTool(QObject* parent = nullptr);

    QString name() const override;
    QString description() const override;

    QWidget* widget() override;

    CaptureTool* copy(QObject* parent = nullptr) override;
};
