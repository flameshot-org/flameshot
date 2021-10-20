// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturetool.h"

class AbstractActionTool : public CaptureTool
{
    Q_OBJECT
public:
    explicit AbstractActionTool(QObject* parent = nullptr);

    bool isValid() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;
    QRect boundingRect() const override;

    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void drawStart(const CaptureContext& context) override;
    void onColorChanged(const QColor& c) override;
    void onSizeChanged(int size) override;
};
