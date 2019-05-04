#include "QZXingImageProvider.h"
#include <QDebug>
#include <QUrlQuery>
#include "QZXing.h"

QZXingImageProvider::QZXingImageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage QZXingImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    int slashIndex = id.indexOf('/');
    if (slashIndex == -1)
    {
        qWarning() << "Can't parse url" << id << ". Usage is encode?<format>/<data>";
        return QImage();
    }

    //Detect operation (ex. encode)
    QString operationName = id.left(slashIndex);
    if(operationName != "encode")
    {
        qWarning() << "Operation not supported: " << operationName;
        return QImage();
    }

    QString data;
    QZXing::EncoderFormat format = QZXing::EncoderFormat_QR_CODE;
    QZXing::EncodeErrorCorrectionLevel correctionLevel = QZXing::EncodeErrorCorrectionLevel_L;

    int customSettingsIndex = id.lastIndexOf('?');
    if(customSettingsIndex >= 0)
    {
        int startOfDataIndex = slashIndex + 1;
        data = id.mid(startOfDataIndex, customSettingsIndex - (startOfDataIndex));

        //The dummy option has been added due to a bug(?) of QUrlQuery
        // it could not recognize the first key-value pair provided
        QUrlQuery optionQuery("options?dummy=&" + id.mid(customSettingsIndex + 1));

        QString correctionLevelString = optionQuery.queryItemValue("corretionLevel");
        QString formatString = optionQuery.queryItemValue("format");
        if(formatString != "qrcode")
        {
            qWarning() << "Format not supported: " << formatString;
            return QImage();
        }

        if(correctionLevelString == "H")
            correctionLevel = QZXing::EncodeErrorCorrectionLevel_H;
        else if(correctionLevelString == "Q")
            correctionLevel = QZXing::EncodeErrorCorrectionLevel_Q;
        else if(correctionLevelString == "M")
            correctionLevel = QZXing::EncodeErrorCorrectionLevel_M;
        else if(correctionLevelString == "L")
            correctionLevel = QZXing::EncodeErrorCorrectionLevel_L;
    } else
    {
        data = id.mid(slashIndex + 1);
    }

    QImage result = QZXing::encodeData(data, format, requestedSize, correctionLevel);
    *size = result.size();
    return result;
}
