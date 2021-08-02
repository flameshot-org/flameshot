// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/abstractactiontool.h"

class MoveTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit MoveTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    CaptureTool::Type type() const override;
    QString description() const override;
    bool isSelectable() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;

public slots:
    void pressed(CaptureContext& context) override;
};
