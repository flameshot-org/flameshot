// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturetool.h"

class AbstractPathTool : public CaptureTool
{
    Q_OBJECT
public:
    explicit AbstractPathTool(QObject* parent = nullptr);

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;
    QRect mousePreviewRect(const CaptureContext& context) const override;
    QRect boundingRect() const override;
    void move(const QPoint& mousePos) override;
    const QPoint* pos() override;
    int size() const override { return m_thickness; };

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void onColorChanged(const QColor& c) override;
    void onSizeChanged(int size) override;

protected:
    void copyParams(const AbstractPathTool* from, AbstractPathTool* to);
    void addPoint(const QPoint& point);

    // class members
    QRect m_pathArea;
    QColor m_color;
    QVector<QPoint> m_points;
    // use m_padding to extend the area of the backup
    int m_padding;
    QPoint m_pos;

private:
    int m_thickness;
};
