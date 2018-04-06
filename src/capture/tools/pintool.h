#pragma once

#include "capturetool.h"

class PinTool : public CaptureTool
{
    Q_OBJECT
public:
    explicit PinTool(QObject *parent = nullptr);

    int id() const override;
    bool isSelectable() const override;
    ToolWorkType toolType() const override;

    QString iconName() const override;
    QString name() const override;
    QString description() const override;

    void processImage(
            QPainter &painter,
            const QVector<QPoint> &points,
            const QColor &color,
            const int thickness) override;

    void onPressed() override;

private:


};
