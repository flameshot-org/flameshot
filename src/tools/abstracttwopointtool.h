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
    virtual void drawObjectSelection(QPainter& painter) override;

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void drawMoveWithAdjustment(const QPoint& p) override;
    void colorChanged(const QColor& c) override;
    void thicknessChanged(const int th) override;

protected:
    QPair<QPoint, QPoint> m_points;
    QColor m_color;
    int m_thickness;
    // use m_padding to extend the area of the backup
    int m_padding;

    bool m_supportsOrthogonalAdj = false;
    bool m_supportsDiagonalAdj = false;

private:
    QPoint adjustedVector(QPoint v) const;
};
