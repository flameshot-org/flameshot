#include "ImageHandler.h"
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QThread>
#include <QTime>

#if QT_VERSION < 0x050000
    #include <QGraphicsObject>
    #include <QStyleOptionGraphicsItem>
#endif // QT_VERSION < Qt 5.0

#if defined(QZXING_QML)
    #include <QQuickItem>
    #include <QQuickItemGrabResult>
    #include <QQuickWindow>
#endif //QZXING_QML

ImageHandler::ImageHandler(QObject *parent) :
    QObject(parent)
{
}

QImage ImageHandler::extractQImage(QObject *imageObj, int offsetX, int offsetY, int width, int height)
{
    QImage img;
#if defined(QZXING_QML)
#if QT_VERSION >= 0x050000
    QQuickItem *item = qobject_cast<QQuickItem *>(imageObj);

    if (!item || !item->window()->isVisible()) {
        qWarning() << "ImageHandler: item is NULL";
        return QImage();
    }

    QTime timer;
    timer.start();
    QSharedPointer<QQuickItemGrabResult> result = item->grabToImage();
    pendingGrabbersLocker.lockForWrite();
    pendingGrabbers << result.data();
    pendingGrabbersLocker.unlock();

    connect(result.data(), &QQuickItemGrabResult::ready, this, &ImageHandler::imageGrabberReady);
    while (timer.elapsed() < 1000) {
        pendingGrabbersLocker.lockForRead();
        if (!pendingGrabbers.contains(result.data())) {
            pendingGrabbersLocker.unlock();
            break;
        }
        pendingGrabbersLocker.unlock();
        qApp->processEvents();
        QThread::yieldCurrentThread();
    }
    img = result->image();
#else // QT_VERSION >= 0x050000
    QGraphicsObject *item = qobject_cast<QGraphicsObject*>(imageObj);

    if (!item) {
        qWarning() << "ImageHandler: item is NULL";
        return QImage();
    }

    img = QImage(item->boundingRect().size().toSize(), QImage::Format_RGB32);
    img.fill(QColor(255, 255, 255).rgb());
    QPainter painter(&img);
    QStyleOptionGraphicsItem styleOption;
    item->paint(&painter, &styleOption);
#endif // QT_VERSION >= 0x050000
#else // defined(QZXING_QML)
    Q_UNUSED(imageObj);
#endif // defined(QZXING_QML)

    if (offsetX < 0)
        offsetX = 0;
    if (offsetY < 0)
        offsetY = 0;
    if (width < 0)
        width = 0;
    if (height < 0)
        height = 0;

    if (offsetX || offsetY || width || height)
        return img.copy(offsetX, offsetY, width, height);
    else
        return img;
}

void ImageHandler::save(QObject *imageObj, const QString &path,
                        const int offsetX, const int offsetY,
                        const int width, const int height)
{
    QImage img = extractQImage(imageObj, offsetX, offsetY, width, height);
    img.save(path);
}

#if QT_VERSION >= 0x050000
    void ImageHandler::imageGrabberReady()
    {
        pendingGrabbersLocker.lockForWrite();
        pendingGrabbers.remove(sender());
        pendingGrabbersLocker.unlock();
    }
#endif

