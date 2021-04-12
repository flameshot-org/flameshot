// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturetool.h"

class AbstractTwoPointTool : public CaptureTool
{
    Q_OBJECT
public:
    explicit AbstractTwoPointTool(QObject* parent = nullptr);

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;
    void move(const QPoint& pos) override;
    const QPoint* pos() override;
    void drawObjectSelection(QPainter& painter) override;
    int thickness() override { return m_thickness; };
    const QColor& color() { return m_color; };
    const QPair<QPoint, QPoint> points() { return m_points; };
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void drawMoveWithAdjustment(const QPoint& p) override;
    void colorChanged(const QColor& c) override;
    void thicknessChanged(int th) override;
    virtual void drawStart(const CaptureContext& context) override;

private:
    QPoint adjustedVector(QPoint v) const;

protected:
    void copyParams(const AbstractTwoPointTool* from, AbstractTwoPointTool* to);
    void setPadding(int padding) { m_padding = padding; };

private:
    // class members
    int m_thickness;
    int m_padding;
    QColor m_color;
    QPair<QPoint, QPoint> m_points;

protected:
    // use m_padding to extend the area of the backup
    bool m_supportsOrthogonalAdj = false;
    bool m_supportsDiagonalAdj = false;
};
