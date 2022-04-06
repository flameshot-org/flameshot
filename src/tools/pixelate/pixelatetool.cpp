#include "pixelatetool.h"
#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <abstractlogger.h>

PixelateTool::PixelateTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_invertSelection = false;
}

QIcon PixelateTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "pixelate.svg");
}

QString PixelateTool::name() const
{
    return tr("Pixelate");
}

CaptureTool::Type PixelateTool::type() const
{
    return CaptureTool::TYPE_PIXELATE;
}

QString PixelateTool::description() const
{
    return tr("Set Pixelate as the paint tool");
}

QRect PixelateTool::boundingRect() const
{
    return QRect(points().first, points().second).normalized();
}

CaptureTool* PixelateTool::copy(QObject* parent)
{
    auto* tool = new PixelateTool(parent);
    copyParams(this, tool);
    tool->m_invertSelection = this->m_invertSelection;
    return tool;
}

void PixelateTool::process(QPainter& painter, const QPixmap& pixmap)
{
    QRect selection = boundingRect().intersected(pixmap.rect());
    if (m_invertSelection) {
        QRegion reverse_selection(pixmap.rect());
        reverse_selection = reverse_selection.subtracted(selection);
        painter.setClipRegion(reverse_selection);
        selection = pixmap.rect();
    }
    auto pixelRatio = pixmap.devicePixelRatio();
    QRect selectionScaled = QRect(selection.topLeft() * pixelRatio,
                                  selection.bottomRight() * pixelRatio);

    // If thickness is less than 1, use old blur process
    if (size() <= 1) {
        auto* blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(10);
        auto* item = new QGraphicsPixmapItem(pixmap.copy(selectionScaled));
        item->setGraphicsEffect(blur);

        QGraphicsScene scene;
        scene.addItem(item);

        scene.render(&painter, selection, QRectF());
        blur->setBlurRadius(12);
        // multiple repeat for make blur effect stronger
        scene.render(&painter, selection, QRectF());

    } else {
        int width =
          static_cast<int>(selection.width() * (0.5 / qMax(1, size() + 1)));
        int height =
          static_cast<int>(selection.height() * (0.5 / qMax(1, size() + 1)));
        QSize size = QSize(qMax(width, 1), qMax(height, 1));

        QPixmap t = pixmap.copy(selectionScaled);
        t = t.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        t = t.scaled(selection.width(), selection.height());
        painter.drawImage(selection, t.toImage());
    }
}

void PixelateTool::drawSearchArea(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.fillRect(boundingRect(), QBrush(Qt::black));
}

void PixelateTool::paintMousePreview(QPainter& painter,
                                     const CaptureContext& context)
{
    Q_UNUSED(context)
    Q_UNUSED(painter)
}

void PixelateTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}

QWidget* PixelateTool::configurationWidget()
{
    if (m_confW == nullptr) {
        m_confW = new PixelateConfig();
        m_confW->setInvertSelection(m_invertSelection);
        connect(m_confW,
                &PixelateConfig::toggleInvertSelection,
                this,
                &PixelateTool::setInvertSelection);
    }
    return m_confW;
}

void PixelateTool::setInvertSelection(bool invert)
{
    m_invertSelection = invert;
}
