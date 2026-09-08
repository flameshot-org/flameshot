// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "tools/abstractactiontool.h"

class QrTool : public AbstractActionTool
{
    Q_OBJECT

public:
    explicit QrTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;
    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    CaptureTool::Type type() const override;
    QString description() const override;
    CaptureTool* copy(QObject* parent = nullptr) override;

    void pressed(CaptureContext& context) override;
};
