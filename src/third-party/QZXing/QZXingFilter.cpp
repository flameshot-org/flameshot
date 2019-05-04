#include "zxing/ZXing.h"
#include "QZXingFilter.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
//#include "QZXingImageProvider.h"

namespace {
    uchar gray(uchar r, uchar g, uchar b)
    {
        return (306 * (r & 0xFF) +
                601 * (g & 0xFF) +
                117 * (b & 0xFF) +
                0x200) >> 10;
    }
    uchar yuvToGray(uchar Y, uchar U, uchar V)
    {
        const int C = int(Y) - 16;
        const int D = int(U) - 128;
        const int E = int(V) - 128;
        return gray(
            qBound<uchar>(0, uchar((298 * C + 409 * E + 128) >> 8), 255),
            qBound<uchar>(0, uchar((298 * C - 100 * D - 208 * E + 128) >> 8), 255),
            qBound<uchar>(0, uchar((298 * C + 516 * D + 128) >> 8), 255)
        );
    }
}

QZXingFilter::QZXingFilter(QObject *parent)
    : QAbstractVideoFilter(parent)
    , decoding(false)
{
    /// Connecting signals to handlers that will send signals to QML
    connect(&decoder, &QZXing::decodingStarted,
            this, &QZXingFilter::handleDecodingStarted);
    connect(&decoder, &QZXing::decodingFinished,
            this, &QZXingFilter::handleDecodingFinished);
}

QZXingFilter::~QZXingFilter()
{
    if(!processThread.isFinished()) {
      processThread.cancel();
      processThread.waitForFinished();
    }
}

void QZXingFilter::handleDecodingStarted()
{
    decoding = true;
    emit decodingStarted();
    emit isDecodingChanged();
}

void QZXingFilter::handleDecodingFinished(bool succeeded)
{
    decoding = false;
    emit decodingFinished(succeeded, decoder.getProcessTimeOfLastDecoding());
    emit isDecodingChanged();
}

QVideoFilterRunnable * QZXingFilter::createFilterRunnable()
{
    return new QZXingFilterRunnable(this);
}

///
/// QZXingFilterRunnable
///

QZXingFilterRunnable::QZXingFilterRunnable(QZXingFilter * filter)
    : QObject(ZXING_NULLPTR)
    , filter(filter)
{

}
QZXingFilterRunnable::~QZXingFilterRunnable()
{
    filter = ZXING_NULLPTR;
}

QVideoFrame QZXingFilterRunnable::run(QVideoFrame * input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags)
{
    Q_UNUSED(surfaceFormat);
    Q_UNUSED(flags);

    /// We dont want to decode every single frame we get, as this would be very costly
    /// These checks are attempt on having only 1 frame being processed at a time.
    if(!input || !input->isValid())
    {
        //qDebug() << "[QZXingFilterRunnable] Invalid Input ";
        return * input;
    }
    if(filter->isDecoding())
    {
        //qDebug() << "------ decoder busy.";
        return * input;
    }
    if(!filter->processThread.isFinished())
    {
        //qDebug() << "--[]-- decoder busy.";
        return * input;
    }

    filter->decoding = true;

    /// Copy the data we need to the filter.
	/// TODO: Depending on the system / platform, this copy hangs up the UI for some seconds. Fix this.
    filter->frame.copyData(* input);

    /// All processing that has to happen in another thread, as we are now in the UI thread.
    filter->processThread = QtConcurrent::run(this, &QZXingFilterRunnable::processVideoFrameProbed, filter->frame, filter->captureRect.toRect());

    return * input;
}

static bool isRectValid(const QRect& rect)
{
  return rect.x() >= 0 && rect.y() >= 0 && rect.isValid();
}

struct CaptureRect
{
    CaptureRect(const QRect& captureRect, int sourceWidth, int sourceHeight)
        : isValid(isRectValid(captureRect))
        , sourceWidth(sourceWidth)
        , sourceHeight(sourceHeight)
        , startX(isValid ? captureRect.x() : 0)
        , targetWidth(isValid ? captureRect.width() : sourceWidth)
        , endX(startX + targetWidth)
        , startY(isValid ? captureRect.y() : 0)
        , targetHeight(isValid ? captureRect.height() : sourceHeight)
        , endY(startY + targetHeight)
    {}

    bool isValid;
    char pad[3]; // avoid warning about padding

    int sourceWidth;
    int sourceHeight;

    int startX;
    int targetWidth;
    int endX;

    int startY;
    int targetHeight;
    int endY;
};

static QImage* rgbDataToGrayscale(const uchar* data, const CaptureRect& captureRect,
                                 const int alpha, const int red,
                                 const int green, const int blue,
                                 const bool isPremultiplied = false)
{
    const int stride = (alpha < 0) ? 3 : 4;

    const int endX = captureRect.sourceWidth - captureRect.startX - captureRect.targetWidth;
    const int skipX = (endX + captureRect.startX) * stride;

    QImage *image_ptr = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
    uchar* pixelInit = image_ptr->bits();
    data += (captureRect.startY * captureRect.sourceWidth + captureRect.startX) * stride;
    for (int y = 1; y <= captureRect.targetHeight; ++y) {

    //Quick fix for iOS devices. Will be handled better in the future
#ifdef Q_OS_IOS
        uchar* pixel = pixelInit + (y - 1) * captureRect.targetWidth;
#else
        uchar* pixel = pixelInit + (captureRect.targetHeight - y) * captureRect.targetWidth;
#endif
        for (int x = 0; x < captureRect.targetWidth; ++x) {
            uchar r = data[red];
            uchar g = data[green];
            uchar b = data[blue];
            if (isPremultiplied) {
                uchar a = data[alpha];
                r = uchar((uint(r) * 255) / a);
                g = uchar((uint(g) * 255) / a);
                b = uchar((uint(b) * 255) / a);
            }
            *pixel = gray(r, g, b);
            ++pixel;
            data += stride;
        }
        data += skipX;
    }

    return image_ptr;
}

void QZXingFilterRunnable::processVideoFrameProbed(SimpleVideoFrame & videoFrame, const QRect& _captureRect)
{
    if (videoFrame.data.length() < 1) {
        qDebug() << "QZXingFilterRunnable: Buffer is empty";
        filter->decoding = false;
        return;
    }

    static unsigned int i = 0; i++;
//    qDebug() << "Future: Going to process frame: " << i;

    const int width = videoFrame.size.width();
    const int height = videoFrame.size.height();
    const CaptureRect captureRect(_captureRect, width, height);
    const uchar* data = reinterpret_cast<const uchar *>(videoFrame.data.constData());

    uchar* pixel;
    int wh;
    int w_2;
    int wh_54;

    const uint32_t *yuvPtr = reinterpret_cast<const uint32_t *>(data);

    /// Create QImage from QVideoFrame.
    QImage *image_ptr = ZXING_NULLPTR;

    switch (videoFrame.pixelFormat) {
    case QVideoFrame::Format_RGB32:
        image_ptr = rgbDataToGrayscale(data, captureRect, 0, 1, 2, 3);
        break;
    case QVideoFrame::Format_ARGB32:
        image_ptr = rgbDataToGrayscale(data, captureRect, 0, 1, 2, 3);
	break;
    case QVideoFrame::Format_ARGB32_Premultiplied:
        image_ptr = rgbDataToGrayscale(data, captureRect, 0, 1, 2, 3, true);
        break;
    case QVideoFrame::Format_BGRA32:
        image_ptr = rgbDataToGrayscale(data, captureRect, 3, 2, 1, 0);
        break;
    case QVideoFrame::Format_BGRA32_Premultiplied:
        image_ptr = rgbDataToGrayscale(data, captureRect, 3, 2, 1, 0, true);
        break;
    case QVideoFrame::Format_BGR32:
        image_ptr = rgbDataToGrayscale(data, captureRect, 3, 2, 1, 0);
        break;
    case QVideoFrame::Format_BGR24:
        image_ptr = rgbDataToGrayscale(data, captureRect, -1, 2, 1, 0);
        break;
    case QVideoFrame::Format_BGR555:
        /// This is a forced "conversion", colors end up swapped.
        image_ptr = new QImage(data, width, height, QImage::Format_RGB555);
        break;
    case QVideoFrame::Format_BGR565:
        /// This is a forced "conversion", colors end up swapped.
        image_ptr = new QImage(data, width, height, QImage::Format_RGB16);
        break;
    case QVideoFrame::Format_YUV420P:
        //fix for issues #4 and #9
        image_ptr = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
        pixel = image_ptr->bits();
        wh = width * height;
        w_2 = width / 2;
        wh_54 = wh * 5 / 4;

        for (int y = captureRect.startY; y < captureRect.endY; y++) {
            const int Y_offset = y * width;
            const int y_2 = y / 2;
            const int U_offset = y_2 * w_2 + wh;
            const int V_offset = y_2 * w_2 + wh_54;
            for (int x = captureRect.startX; x < captureRect.endX; x++) {
                const int x_2 = x / 2;
                const uchar Y = data[Y_offset + x];
                const uchar U = data[U_offset + x_2];
                const uchar V = data[V_offset + x_2];
                *pixel = yuvToGray(Y, U, V);
                ++pixel;
            }
        }
        break;
    case QVideoFrame::Format_NV12:
        /// nv12 format, encountered on macOS
        image_ptr = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
        pixel = image_ptr->bits();
        wh = width * height;
        w_2 = width / 2;
        wh_54 = wh * 5 / 4;

        for (int y = captureRect.startY; y < captureRect.endY; y++) {
            const int Y_offset = y * width;
            const int y_2 = y / 2;
            const int U_offset = y_2 * w_2 + wh;
            const int V_offset = y_2 * w_2 + wh_54;
            for (int x = captureRect.startX; x < captureRect.endX; x++) {
                const int x_2 = x / 2;
                const uchar Y = data[Y_offset + x];
                const uchar U = data[U_offset + x_2];
                const uchar V = data[V_offset + x_2];
                *pixel = yuvToGray(Y, U, V);
                ++pixel;
            }
        }
        break;
    case QVideoFrame::Format_YUYV:
        image_ptr = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
        pixel = image_ptr->bits();

        for (int y = captureRect.startY; y < captureRect.endY; y++){
            const uint32_t *row = &yuvPtr[y*(width/2)-(width/4)];
            for (int x = captureRect.startX; x < captureRect.endX; x++){
                const uint8_t *pxl = reinterpret_cast<const uint8_t *>(&row[x]);
                const uint8_t y0 = pxl[0];
                const uint8_t u  = pxl[1];
                const uint8_t v  = pxl[3];
                *pixel = yuvToGray(y0, u, v);
                ++pixel;
            }
        }
        break;
        /// TODO: Handle (create QImages from) YUV formats.
    default:
        QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(videoFrame.pixelFormat);
        image_ptr = new QImage(data, width, height, imageFormat);
        break;
    }

    if(!image_ptr || image_ptr->isNull())
    {
        qDebug() << "QZXingFilterRunnable error: Cant create image file to process.";
        qDebug() << "Maybe it was a format conversion problem? ";
        qDebug() << "VideoFrame format: " << videoFrame.pixelFormat;
        qDebug() << "Image corresponding format: " << QVideoFrame::imageFormatFromPixelFormat(videoFrame.pixelFormat);
        filter->decoding = false;
        return;
    }

    if (captureRect.isValid && image_ptr->size() != _captureRect.size())
        image_ptr = new QImage(image_ptr->copy(_captureRect));

    //qDebug() << "image.size()" << image_ptr->size();
    //qDebug() << "image.format()" << image_ptr->format();
    //qDebug() << "videoFrame.pixelFormat" << videoFrame.pixelFormat;
    //const QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/qrtest/test_" + QString::number(i % 100) + ".png";
    //qDebug() << "saving image" << i << "at:" << path << image_ptr->save(path);

    //QZXingImageProvider::getInstance()->storeImage(image);

    decode(*image_ptr);

    delete image_ptr;
}

QString QZXingFilterRunnable::decode(const QImage &image)
{
    return (filter != ZXING_NULLPTR) ?
      filter->decoder.decodeImage(image, image.width(), image.height()) : QString();
}
