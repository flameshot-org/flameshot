// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/abstracttwopointtool.h"

class CircleCountTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit CircleCountTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QString info() override;
    bool isValid() const override;

    QRect mousePreviewRect(const CaptureContext& context) const override;
    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

protected:
    CaptureTool::Type type() const override;
    void copyParams(const CircleCountTool* from, CircleCountTool* to);

public slots:
    void drawStart(const CaptureContext& context) override;
    void pressed(CaptureContext& context) override;

private:
    QString m_tempString;
    bool m_valid;
};
