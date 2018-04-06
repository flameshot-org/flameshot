#include "src/capture/workers/pin/pinwidget.h"
#include "src/capture/workers/pin/contentwidget.h"
#include "src/capture/widgets/capturewidget.h"
#include "pintool.h"

PinTool::PinTool(QObject *parent) : CaptureTool(parent) {

}

int PinTool::id() const {
    return 0;
}

bool PinTool::isSelectable() const {
    return true;
}

QString PinTool::iconName() const {
    return "push-pin.png";
}

QString PinTool::name() const {
    return tr("Pin");
}

QString PinTool::description() const {
    return tr("Paste images on the desktop");
}

CaptureTool::ToolWorkType PinTool::toolType() const {
    return TYPE_WORKER;
}

void PinTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color,
        const int thickness)
{
    Q_UNUSED(painter);
    Q_UNUSED(points);
    Q_UNUSED(color);
    Q_UNUSED(thickness);
}

void PinTool::onPressed() {

    Q_EMIT requestAction(REQ_PIN);

    auto *capture = new CaptureWidget();
    auto *contentWidget = new ContentWidget();
    auto *window = new PinWidget(contentWidget);

    QRect pos = QRect (capture->capturePos.left() - MARGIN,
                       capture->capturePos.top() - MARGIN,
                       capture->capturePos.width() + 2 * MARGIN,
                       capture->capturePos.height() + 2 * MARGIN);

    window->setGeometry(pos);
    window->show();

}

