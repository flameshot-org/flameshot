#include "QZXing.h"

#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/Binarizer.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/DecodeHints.h>
#include "CameraImageWrapper.h"
#include "ImageHandler.h"
#include <QTime>
#include <QUrl>
#include <QFileInfo>
#include <zxing/qrcode/encoder/Encoder.h>
#include <zxing/qrcode/ErrorCorrectionLevel.h>
#include <zxing/common/detector/WhiteRectangleDetector.h>
#include <QColor>
#include <QtCore/QTextCodec>
#include <QDebug>

#ifdef QZXING_QML
#if QT_VERSION >= 0x040700 && QT_VERSION < 0x050000
#include <QtDeclarative>
#elif QT_VERSION >= 0x050000
#include <QtQml/qqml.h>
#endif
#endif //QZXING_QML

#ifdef QZXING_MULTIMEDIA
#include "QZXingFilter.h"
#endif //QZXING_MULTIMEDIA

#ifdef QZXING_QML
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include "QZXingImageProvider.h"
#endif //QZXING_QML


using namespace zxing;

QZXing::QZXing(QObject *parent) : QObject(parent), tryHarder_(false), lastDecodeOperationSucceded_(false)
{
    decoder = new MultiFormatReader();
    setDecoder(DecoderFormat_QR_CODE |
               DecoderFormat_DATA_MATRIX |
               DecoderFormat_UPC_E |
               DecoderFormat_UPC_A |
               DecoderFormat_UPC_EAN_EXTENSION |
               DecoderFormat_RSS_14 |
               DecoderFormat_RSS_EXPANDED |
               DecoderFormat_PDF_417 |
               DecoderFormat_MAXICODE |
               DecoderFormat_EAN_8 |
               DecoderFormat_EAN_13 |
               DecoderFormat_CODE_128 |
               DecoderFormat_CODE_93 |
               DecoderFormat_CODE_39 |
               DecoderFormat_CODABAR |
               DecoderFormat_ITF |
               DecoderFormat_Aztec);
    imageHandler = new ImageHandler();
}

QZXing::~QZXing()
{
    if (imageHandler)
        delete imageHandler;

    if (decoder)
        delete decoder;
}

QZXing::QZXing(QZXing::DecoderFormat decodeHints, QObject *parent) : QObject(parent), lastDecodeOperationSucceded_(false)
{
    decoder = new MultiFormatReader();
    imageHandler = new ImageHandler();

    setDecoder(decodeHints);
}

#ifdef QZXING_QML

#if QT_VERSION >= 0x040700
void QZXing::registerQMLTypes()
{
    qmlRegisterType<QZXing>("QZXing", 2, 3, "QZXing");

#ifdef QZXING_MULTIMEDIA
    qmlRegisterType<QZXingFilter>("QZXing", 2, 3, "QZXingFilter");
#endif //QZXING_MULTIMEDIA

}
#endif //QT_VERSION >= Qt 4.7

#if  QT_VERSION >= 0x050000
void QZXing::registerQMLImageProvider(QQmlEngine& engine)
{
    engine.addImageProvider(QLatin1String("QZXing"), new QZXingImageProvider());
}
#endif //QT_VERSION >= Qt 5.0

#endif //QZXING_QML

void QZXing::setTryHarder(bool tryHarder)
{
    tryHarder_ = tryHarder;
}

bool QZXing::getTryHarder()
{
    return tryHarder_;
}

QString QZXing::decoderFormatToString(int fmt)
{
    switch (fmt) {
    case DecoderFormat_Aztec:
        return "AZTEC";

    case DecoderFormat_CODABAR:
        return "CODABAR";

    case DecoderFormat_CODE_39:
        return "CODE_39";

    case DecoderFormat_CODE_93:
        return "CODE_93";

    case DecoderFormat_CODE_128:
        return "CODE_128";

    case DecoderFormat_CODE_128_GS1:
        return "CODE_128_GS1";

    case DecoderFormat_DATA_MATRIX:
        return "DATA_MATRIX";

    case DecoderFormat_EAN_8:
        return "EAN_8";

    case DecoderFormat_EAN_13:
        return "EAN_13";

    case DecoderFormat_ITF:
        return "ITF";

    case DecoderFormat_MAXICODE:
        return "MAXICODE";

    case DecoderFormat_PDF_417:
        return "PDF_417";

    case DecoderFormat_QR_CODE:
        return "QR_CODE";

    case DecoderFormat_RSS_14:
        return "RSS_14";

    case DecoderFormat_RSS_EXPANDED:
        return "RSS_EXPANDED";

    case DecoderFormat_UPC_A:
        return "UPC_A";

    case DecoderFormat_UPC_E:
        return "UPC_E";

    case DecoderFormat_UPC_EAN_EXTENSION:
        return "UPC_EAN_EXTENSION";
    } // switch
    return QString();
}

QString QZXing::foundedFormat() const
{
    return foundedFmt;
}

QString QZXing::charSet() const
{
    return charSet_;
}

bool QZXing::getLastDecodeOperationSucceded()
{
    return lastDecodeOperationSucceded_;
}

void QZXing::setDecoder(const uint &hint)
{
    unsigned int newHints = 0;

    if(hint & DecoderFormat_Aztec)
        newHints |= DecodeHints::AZTEC_HINT;

    if(hint & DecoderFormat_CODABAR)
        newHints |= DecodeHints::CODABAR_HINT;

    if(hint & DecoderFormat_CODE_39)
        newHints |= DecodeHints::CODE_39_HINT;

    if(hint & DecoderFormat_CODE_93)
        newHints |= DecodeHints::CODE_93_HINT;

    if(hint & DecoderFormat_CODE_128)
        newHints |= DecodeHints::CODE_128_HINT;

    if(hint & DecoderFormat_DATA_MATRIX)
        newHints |= DecodeHints::DATA_MATRIX_HINT;

    if(hint & DecoderFormat_EAN_8)
        newHints |= DecodeHints::EAN_8_HINT;

    if(hint & DecoderFormat_EAN_13)
        newHints |= DecodeHints::EAN_13_HINT;

    if(hint & DecoderFormat_ITF)
        newHints |= DecodeHints::ITF_HINT;

    if(hint & DecoderFormat_MAXICODE)
        newHints |= DecodeHints::MAXICODE_HINT;

    if(hint & DecoderFormat_PDF_417)
        newHints |= DecodeHints::PDF_417_HINT;

    if(hint & DecoderFormat_QR_CODE)
        newHints |= DecodeHints::QR_CODE_HINT;

    if(hint & DecoderFormat_RSS_14)
        newHints |= DecodeHints::RSS_14_HINT;

    if(hint & DecoderFormat_RSS_EXPANDED)
        newHints |= DecodeHints::RSS_EXPANDED_HINT;

    if(hint & DecoderFormat_UPC_A)
        newHints |= DecodeHints::UPC_A_HINT;

    if(hint & DecoderFormat_UPC_E)
        newHints |= DecodeHints::UPC_E_HINT;

    if(hint & DecoderFormat_UPC_EAN_EXTENSION)
        newHints |= DecodeHints::UPC_EAN_EXTENSION_HINT;

    if(hint & DecoderFormat_CODE_128_GS1)
    {
        newHints |= DecodeHints::CODE_128_HINT;
        newHints |= DecodeHints::ASSUME_GS1;
    }

    enabledDecoders = newHints;

    emit enabledFormatsChanged();
}

/*!
 * \brief getTagRec - returns rectangle containing the tag.
 *
 * To be able display tag rectangle regardless of the size of the bit matrix rect is in related coordinates [0; 1].
 * \param resultPoints
 * \param bitMatrix
 * \return
 */
QRectF getTagRect(const ArrayRef<Ref<ResultPoint> > &resultPoints, const Ref<BitMatrix> &bitMatrix)
{
    if (resultPoints->size() < 2)
        return QRectF();

    int matrixWidth = bitMatrix->getWidth();
    int matrixHeight = bitMatrix->getHeight();
    // 1D barcode
    if (resultPoints->size() == 2) {
        WhiteRectangleDetector detector(bitMatrix);
        std::vector<Ref<ResultPoint> > resultRectPoints = detector.detect();

        if (resultRectPoints.size() != 4)
            return QRectF();

        qreal xMin = qreal(resultPoints[0]->getX());
        qreal xMax = xMin;
        for (int i = 1; i < resultPoints->size(); ++i) {
            qreal x = qreal(resultPoints[i]->getX());
            if (x < xMin)
                xMin = x;
            if (x > xMax)
                xMax = x;
        }

        qreal yMin = qreal(resultRectPoints[0]->getY());
        qreal yMax = yMin;
        for (unsigned int i = 1; i < resultRectPoints.size(); ++i) {
            qreal y = qreal(resultRectPoints[i]->getY());
            if (y < yMin)
                yMin = y;
            if (y > yMax)
                yMax = y;
        }

        return QRectF(QPointF(xMin / matrixWidth, yMax / matrixHeight), QPointF(xMax / matrixWidth, yMin / matrixHeight));
    }

    // 2D QR code
    if (resultPoints->size() == 4) {
        qreal xMin = qreal(resultPoints[0]->getX());
        qreal xMax = xMin;
        qreal yMin = qreal(resultPoints[0]->getY());
        qreal yMax = yMin;
        for (int i = 1; i < resultPoints->size(); ++i) {
            qreal x = qreal(resultPoints[i]->getX());
            qreal y = qreal(resultPoints[i]->getY());
            if (x < xMin)
                xMin = x;
            if (x > xMax)
                xMax = x;
            if (y < yMin)
                yMin = y;
            if (y > yMax)
                yMax = y;
        }

        return QRectF(QPointF(xMin / matrixWidth, yMax / matrixHeight), QPointF(xMax / matrixWidth, yMin / matrixHeight));
    }

    return QRectF();
}

QString QZXing::decodeImage(const QImage &image, int maxWidth, int maxHeight, bool smoothTransformation)
{
    QTime t;
    t.start();
    processingTime = -1;
    Ref<Result> res;
    emit decodingStarted();

    if(image.isNull())
    {
        emit decodingFinished(false);
        processingTime = t.elapsed();
        return "";
    }

    CameraImageWrapper *ciw = ZXING_NULLPTR;

    if ((maxWidth > 0) || (maxHeight > 0))
        ciw = CameraImageWrapper::Factory(image, maxWidth, maxHeight, smoothTransformation);
    else
        ciw = CameraImageWrapper::Factory(image, 999, 999, true);

    QString errorMessage = "Unknown";
    try {
        Ref<LuminanceSource> imageRef(ciw);
        Ref<GlobalHistogramBinarizer> binz( new GlobalHistogramBinarizer(imageRef) );
        Ref<BinaryBitmap> bb( new BinaryBitmap(binz) );

        DecodeHints hints(static_cast<DecodeHintType>(enabledDecoders));

        lastDecodeOperationSucceded_ = false;
        try {
            res = decoder->decode(bb, hints);
            processingTime = t.elapsed();
            lastDecodeOperationSucceded_ = true;
        } catch(zxing::Exception &/*e*/){}

        if(!lastDecodeOperationSucceded_)
        {
            hints.setTryHarder(true);

            try {
                res = decoder->decode(bb, hints);
                processingTime = t.elapsed();
                lastDecodeOperationSucceded_ = true;
            } catch(zxing::Exception &/*e*/) {}

            if (tryHarder_ && bb->isRotateSupported()) {
                Ref<BinaryBitmap> bbTmp = bb;

                for (int i=0; (i<3 && !lastDecodeOperationSucceded_); i++) {
                    Ref<BinaryBitmap> rotatedImage(bbTmp->rotateCounterClockwise());
                    bbTmp = rotatedImage;

                    try {
                        res = decoder->decode(rotatedImage, hints);
                        processingTime = t.elapsed();
                        lastDecodeOperationSucceded_ = true;
                    } catch(zxing::Exception &/*e*/) {}
                }
            }
        }

        if (lastDecodeOperationSucceded_) {
            QString string = QString(res->getText()->getText().c_str());
            if (!string.isEmpty() && (string.length() > 0)) {
                int fmt = res->getBarcodeFormat().value;
                foundedFmt = decoderFormatToString(1<<fmt);
                charSet_ = QString::fromStdString(res->getCharSet());
                if (!charSet_.isEmpty()) {
                    QTextCodec *codec = QTextCodec::codecForName(res->getCharSet().c_str());
                    if (codec)
                        string = codec->toUnicode(res->getText()->getText().c_str());
                }

                emit tagFound(string);
                emit tagFoundAdvanced(string, foundedFmt, charSet_);

                try {
                    const QRectF rect = getTagRect(res->getResultPoints(), binz->getBlackMatrix());
                    emit tagFoundAdvanced(string, foundedFmt, charSet_, rect);
                }catch(zxing::Exception &/*e*/){}
            }
            emit decodingFinished(true);
            return string;
        }
    }
    catch(zxing::Exception &e)
    {
        errorMessage = QString(e.what());
    }

    emit error(errorMessage);
    emit decodingFinished(false);
    processingTime = t.elapsed();
    return "";
}

QString QZXing::decodeImageFromFile(const QString& imageFilePath, int maxWidth, int maxHeight, bool smoothTransformation)
{
    // used to have a check if this image exists
    // but was removed because if the image file path doesn't point to a valid image
    // then the QImage::isNull will return true and the decoding will fail eitherway.
    const QString header = "file://";

    QString filePath = imageFilePath;
    if(imageFilePath.startsWith(header))
        filePath = imageFilePath.right(imageFilePath.size() - header.size());

    QUrl imageUrl = QUrl::fromLocalFile(filePath);
    QImage tmpImage = QImage(imageUrl.toLocalFile());
    return decodeImage(tmpImage, maxWidth, maxHeight, smoothTransformation);
}

QString QZXing::decodeImageQML(QObject *item)
{
    return decodeSubImageQML(item);
}

QString QZXing::decodeSubImageQML(QObject *item,
                                  const int offsetX, const int offsetY,
                                  const int width, const int height)
{
    if(item  == ZXING_NULLPTR)
    {
        processingTime = 0;
        emit decodingFinished(false);
        return "";
    }

    QImage img = imageHandler->extractQImage(item, offsetX, offsetY, width, height);

    return decodeImage(img);
}

QString QZXing::decodeImageQML(const QUrl &imageUrl)
{
    return decodeSubImageQML(imageUrl);
}

QString QZXing::decodeSubImageQML(const QUrl &imageUrl,
                                  const int offsetX, const int offsetY,
                                  const int width, const int height)
{
#ifdef QZXING_QML

    QString imagePath = imageUrl.path();
    imagePath = imagePath.trimmed();
    QImage img;
    if (imageUrl.scheme() == "image") {
        if (imagePath.startsWith("/"))
            imagePath = imagePath.right(imagePath.length() - 1);
        QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
        QQuickImageProvider *imageProvider = static_cast<QQuickImageProvider *>(engine->imageProvider(imageUrl.host()));
        QSize imgSize;
        img = imageProvider->requestImage(imagePath, &imgSize, QSize());
    } else {
        QFileInfo fileInfo(imagePath);
        if (!fileInfo.exists()) {
            qDebug() << "[decodeSubImageQML()] The file" << imagePath << "does not exist.";
            emit decodingFinished(false);
            return "";
        }
        img = QImage(imagePath);
    }

    if (offsetX || offsetY || width || height)
        img = img.copy(offsetX, offsetY, width, height);
    return decodeImage(img);
#else
    Q_UNUSED(imageUrl);
    Q_UNUSED(offsetX);
    Q_UNUSED(offsetY);
    Q_UNUSED(width);
    Q_UNUSED(height);
    return decodeImage(QImage());
#endif //QZXING_QML
}

QImage QZXing::encodeData(const QString& data,
                          const EncoderFormat encoderFormat,
                          const QSize encoderImageSize,
                          const EncodeErrorCorrectionLevel errorCorrectionLevel)
{
    QImage image;

    try {
        switch (encoderFormat) {
        case EncoderFormat_QR_CODE:
        {
            Ref<qrcode::QRCode> barcode = qrcode::Encoder::encode(
                        data.toStdString(),
                        errorCorrectionLevel == EncodeErrorCorrectionLevel_H ?
                            qrcode::ErrorCorrectionLevel::H :
                        (errorCorrectionLevel == EncodeErrorCorrectionLevel_Q ?
                            qrcode::ErrorCorrectionLevel::Q :
                        (errorCorrectionLevel == EncodeErrorCorrectionLevel_M ?
                             qrcode::ErrorCorrectionLevel::M :
                             qrcode::ErrorCorrectionLevel::L)));

            Ref<qrcode::ByteMatrix> bytesRef = barcode->getMatrix();
            const std::vector< std::vector <zxing::byte> >& bytes = bytesRef->getArray();
            image = QImage(int(bytesRef->getWidth()), int(bytesRef->getHeight()), QImage::Format_ARGB32);
            for(size_t i=0; i<bytesRef->getWidth(); i++)
                for(size_t j=0; j<bytesRef->getHeight(); j++)
                    image.setPixel(int(i), int(j), bytes[j][i] ?
                                       qRgb(0,0,0) :
                                       qRgb(255,255,255));

            image = image.scaled(encoderImageSize);
            break;
        }
        case EncoderFormat_INVALID:
            break;
        }
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return image;
}

int QZXing::getProcessTimeOfLastDecoding()
{
    return processingTime;
}

uint QZXing::getEnabledFormats() const
{
    return enabledDecoders;
}
