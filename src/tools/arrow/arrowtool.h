// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/abstracttwopointtool.h"
#include <QPainter>
#include <QPainterPath>

class ArrowTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit ArrowTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void drawObjectSelection(QPainter& painter) override;

protected:
    void copyParams(const ArrowTool* from, ArrowTool* to);
    ToolType nameID() const override;

public slots:
    void pressed(const CaptureContext& context) override;

private:
    QPainterPath m_arrowPath;
};
