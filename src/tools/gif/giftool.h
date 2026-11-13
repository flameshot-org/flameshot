// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

#include "tools/abstractactiontool.h"

class GifTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit GifTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;
};
