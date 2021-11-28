#pragma once

#include "src/tools/abstracttwopointtool.h"
#include <QPainter>
#include <QPainterPath>

class CurvedArrowTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit CurvedArrowTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;

protected:
    void copyParams(const CurvedArrowTool* from, CurvedArrowTool* to);
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;

private:
    QPainterPath m_arrowPath;
};
