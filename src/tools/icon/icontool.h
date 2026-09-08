// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "tools/abstracttwopointtool.h"

#include <QPointer>

class IconConfig;

class IconTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit IconTool(QObject* parent = nullptr);
    ~IconTool() override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QString info() override;
    bool isValid() const override;
    QRect mousePreviewRect(const CaptureContext& context) const override;
    QRect boundingRect() const override;
    QWidget* configurationWidget() override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

protected:
    CaptureTool::Type type() const override;
    void copyParams(const IconTool* from, IconTool* to);

public slots:
    void drawStart(const CaptureContext& context) override;
    void pressed(CaptureContext& context) override;

private slots:
    void updateSymbol(const QString& symbol);

private:
    QRect iconRect() const;
    int iconPixelSize() const;
    void drawIcon(QPainter& painter, const QRect& rect, qreal opacity = 1.0);
    void closeConfig();

    QString m_symbol;
    bool m_valid;
    QPointer<IconConfig> m_confW;
    QString m_tempString;
};
