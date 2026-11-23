// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "tools/abstracttwopointtool.h"

#include <QImage>

class PixelateTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit PixelateTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void drawSearchArea(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;

private:
    QImage m_cachedImage;
    QRect m_cachedSelectionScaled;
    int m_cachedSize{ -1 };
    bool m_cachedInsecure{ false };
    qint64 m_cachedPixmapKey{ 0 };
};
