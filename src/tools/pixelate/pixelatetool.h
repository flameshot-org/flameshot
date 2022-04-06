#pragma once

#include "pixelateconfig.h"
#include "src/tools/abstracttwopointtool.h"
#include <QPointer>

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
    QWidget* configurationWidget() override;
    bool isInverSelection();

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;
    void setInvertSelection(bool invert);

private:
    QPointer<PixelateConfig> m_confW;
    bool m_invertSelection;
};
