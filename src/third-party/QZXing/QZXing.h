/*
 * Copyright 2011 QZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef QZXING_H
#define QZXING_H

#include "QZXing_global.h"
#include "zxing/ZXing.h"

#include <QObject>
#include <QImage>

#if QT_VERSION >= 0x050000
    class QQmlEngine;
#endif

// forward declaration
namespace zxing {
class MultiFormatReader;
}
class ImageHandler;

/**
  * A class containing a very very small subset of the ZXing library.
  * Created for ease of use.
  *
  * Anyone interested in using more technical stuff
  * from the ZXing library is welcomed to add/edit on free will.
  *
  * Regarding DecoderFormat, by default all of those are enabled
  */
class
        #ifndef DISABLE_LIBRARY_FEATURES
        QZXINGSHARED_EXPORT
        #endif
        QZXing : public QObject {

    Q_OBJECT
    Q_ENUMS(DecoderFormat)
    Q_PROPERTY(int processingTime READ getProcessTimeOfLastDecoding)
    Q_PROPERTY(uint enabledDecoders READ getEnabledFormats WRITE setDecoder NOTIFY enabledFormatsChanged)
    Q_PROPERTY(bool tryHarder READ getTryHarder WRITE setTryHarder)

public:
    /*
     *
     */
    enum DecoderFormat {
        DecoderFormat_None = 0,
        DecoderFormat_Aztec = 1 << 1,
        DecoderFormat_CODABAR = 1 << 2,
        DecoderFormat_CODE_39 = 1 << 3,
        DecoderFormat_CODE_93 = 1 << 4,
        DecoderFormat_CODE_128 = 1 << 5,
        DecoderFormat_DATA_MATRIX = 1 << 6,
        DecoderFormat_EAN_8 = 1 << 7,
        DecoderFormat_EAN_13 = 1 << 8,
        DecoderFormat_ITF = 1 << 9,
        DecoderFormat_MAXICODE = 1 << 10,
        DecoderFormat_PDF_417 = 1 << 11,
        DecoderFormat_QR_CODE = 1 << 12,
        DecoderFormat_RSS_14 = 1 << 13,
        DecoderFormat_RSS_EXPANDED = 1 << 14,
        DecoderFormat_UPC_A = 1 << 15,
        DecoderFormat_UPC_E = 1 << 16,
        DecoderFormat_UPC_EAN_EXTENSION = 1 << 17,
        DecoderFormat_CODE_128_GS1 = 1 << 18
    } ;
    typedef unsigned int DecoderFormatType;

    enum EncoderFormat {
        EncoderFormat_INVALID,
        EncoderFormat_QR_CODE
    };

    enum EncodeErrorCorrectionLevel {
        EncodeErrorCorrectionLevel_L = 0,
        EncodeErrorCorrectionLevel_M,
        EncodeErrorCorrectionLevel_Q,
        EncodeErrorCorrectionLevel_H
    };

    QZXing(QObject *parent = ZXING_NULLPTR);
    ~QZXing();

    QZXing(DecoderFormat decodeHints, QObject *parent = ZXING_NULLPTR);

#ifdef QZXING_QML

#if QT_VERSION >= 0x040700
    static void registerQMLTypes();
#endif //QT_VERSION >= Qt 4.7

#if  QT_VERSION >= 0x050000
    static void registerQMLImageProvider(QQmlEngine& view);
#endif //QT_VERSION >= Qt 5.0

#endif //QZXING_QML

    void setTryHarder(bool tryHarder);
    bool getTryHarder();
    static QString decoderFormatToString(int fmt);
    Q_INVOKABLE QString foundedFormat() const;
    Q_INVOKABLE QString charSet() const;

    bool getLastDecodeOperationSucceded();

public slots:
    /**
      * The decoding function. Will try to decode the given image based on the enabled decoders.
      * If the image width is larger than maxWidth or image height is larger
      * than maxHeight then the image will be scaled down. Either way, in case of scaling, the aspect
      * ratio of the image will be kept.
      *
      * The smoothTransformation flag determines whether the transformation will be smooth or fast.
      * Smooth transformation provides better results but fast transformation is...faster.
      */
    QString decodeImage(const QImage &image, int maxWidth = -1, int maxHeight = -1, bool smoothTransformation = false);

    /**
      * The decoding function. Will try to decode the given image based on the enabled decoders.
      * The input image is read from a local image file.
      */
    QString decodeImageFromFile(const QString& imageFilePath, int maxWidth=-1, int maxHeight=-1, bool smoothTransformation = false);
    /**
     * The decoding function accessible from QML. (Suggested for Qt 4.x)
     */
    QString decodeImageQML(QObject *item);

    /**
     * The decoding function accessible from QML. Able to set the decoding
     * of a portion of the image. (Suggested for Qt 4.x)
     */
    QString decodeSubImageQML(QObject *item,
                              const int offsetX = 0, const int offsetY = 0,
                              const int width = 0, const int height = 0);

    /**
     * The decoding function accessible from QML. (Suggested for Qt 5.x)
     * Can be used to decode image from the Camera element preview by providing
     * the following string: image://camera/preview_1
     */
    QString decodeImageQML(const QUrl &imageUrl);

    /**
     * The decoding function accessible from QML. Able to set the decoding
     * of a portion of the image.
     * Can be used to decode image from the Camera element preview by providing
     * the following string: image://camera/preview_1
     * (Suggested for Qt 5.x)
     */
    QString decodeSubImageQML(const QUrl &imageUrl,
                              const int offsetX = 0, const int offsetY = 0,
                              const int width = 0, const int height = 0);

    /**
     * The main encoding function. Currently supports only Qr code encoding
     */
    static QImage encodeData(const QString& data,
                             const EncoderFormat encoderFormat = EncoderFormat_QR_CODE,
                             const QSize encoderImageSize = QSize(240, 240),
                             const EncodeErrorCorrectionLevel errorCorrectionLevel = EncodeErrorCorrectionLevel_L);

    /**
      * Get the prossecing time in millisecond of the last decode operation.
      * Added mainly as a statistic measure.
      * Decoding operation fails, the processing time equals to -1.
      */
    int getProcessTimeOfLastDecoding();

    /**
      * Get the decoders that are enabled at the moment.
      * Returns a uint which is a bitwise OR of DecoderFormat enumeration values.
      */
    uint getEnabledFormats() const;
    /**
      * Set the enabled decoders.
      * As argument it is possible to pass conjuction of decoders by using logic OR.
      * e.x. setDecoder ( DecoderFormat_QR_CODE | DecoderFormat_EAN_13 | DecoderFormat_CODE_39 )
      */
    void setDecoder(const uint &hint);

signals:
    void decodingStarted();
    void decodingFinished(bool succeeded);
    void enabledFormatsChanged();
    void tagFound(QString tag);
    void tagFoundAdvanced(const QString &tag, const QString &format, const QString &charSet) const;
    void tagFoundAdvanced(const QString &tag, const QString &format, const QString &charSet, const QRectF &rect) const;
    void error(QString msg);

private:
    zxing::MultiFormatReader *decoder;
    DecoderFormatType enabledDecoders;
    ImageHandler *imageHandler;
    int processingTime;
    QString foundedFmt;
    QString charSet_;
    bool tryHarder_;
    bool lastDecodeOperationSucceded_;

    /**
      * If true, the decoding operation will take place at a different thread.
      */
    bool isThreaded;
};

#endif // QZXING_H

