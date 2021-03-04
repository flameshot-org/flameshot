// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2021 Martin Eckleben & Contributors

#pragma once

#include "src/tools/abstractactiontool.h"

class SizeDecreaseTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit SizeDecreaseTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;

protected:
    ToolType nameID() const override;

public slots:
    void pressed(const CaptureContext& context) override;
};
