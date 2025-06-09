// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/abstracttwopointtool.h"

class SelectionTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit SelectionTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;
};
