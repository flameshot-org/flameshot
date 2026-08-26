// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "tools/abstractactiontool.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

class OcrTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit OcrTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool::Type type() const override;
    CaptureTool* copy(QObject* parent = nullptr) override;

public slots:
    void pressed(CaptureContext& context) override;
};
