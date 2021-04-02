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
    void move(const QPoint& mousePos) override;
    const QPoint* pos() override;
    void drawObjectSelection(QPainter& painter) override;
    AbstractPathTool& operator=(const AbstractPathTool& other)
    {
        this->m_points = other.m_points;
        this->m_color = other.m_color;
        this->m_thickness = other.m_thickness;
        this->m_padding = other.m_padding;
        this->m_pos = other.m_pos;

        // TODO - check that it objects are really coping
        // QVector<QPoint> m_points;
        this->m_points = other.m_points;
        return *this;
    }

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void colorChanged(const QColor& c) override;
    void thicknessChanged(const int th) override;

protected:
    void addPoint(const QPoint& point);

    // class members
    QRect m_pathArea;
    QColor m_color;
    QVector<QPoint> m_points;
    int m_thickness;
    // use m_padding to extend the area of the backup
    int m_padding;
    QPoint m_pos;
};
