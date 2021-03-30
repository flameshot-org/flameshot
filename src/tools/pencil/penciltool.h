// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/abstractpathtool.h"

class PencilTool : public AbstractPathTool
{
    Q_OBJECT
public:
    explicit PencilTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;

    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

protected:
    ToolType nameID() const override;

public slots:
    void drawStart(const CaptureContext& context) override;
    void pressed(const CaptureContext& context) override;
};
