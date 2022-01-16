#pragma once

#include <abstracttwopointtool.h>

class CursorTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit CursorTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    bool isValid() const override;

    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;
};
