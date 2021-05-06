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

    void undo(QPixmap& pixmap) override;

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void colorChanged(const QColor& c) override;
    void thicknessChanged(const int th) override;

protected:
    void updateBackup(const QPixmap& pixmap);
    void addPoint(const QPoint& point);
    QRect backupRect(const QPixmap& pixmap) const;

    QPixmap m_pixmapBackup;
    QRect m_backupArea;
    QColor m_color;
    QVector<QPoint> m_points;
    int m_thickness;
    // use m_padding to extend the area of the backup
    int m_padding;
};
