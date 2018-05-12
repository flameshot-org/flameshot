/****************************************************************************
** Copyright (c) 2012-2015 Debao Zhang <hello@debao.me>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/

#include "cvmatandqimage.h"
#include <QImage>
#include <QSysInfo>
#include <QDebug>
#include <cstring>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace QtOcv {
namespace {

/*ARGB <==> BGRA
 */
cv::Mat argb2bgra(const cv::Mat &mat)
{
    Q_ASSERT(mat.channels()==4);

    cv::Mat newMat(mat.rows, mat.cols, mat.type());
    int from_to[] = {0,3, 1,2, 2,1, 3,0};
    cv::mixChannels(&mat, 1, &newMat, 1, from_to, 4);
    return newMat;
}

cv::Mat adjustChannelsOrder(const cv::Mat &srcMat, MatColorOrder srcOrder, MatColorOrder targetOrder)
{
    Q_ASSERT(srcMat.channels()==4);

    if (srcOrder == targetOrder)
        return srcMat.clone();

    cv::Mat desMat;

    if ((srcOrder == MCO_ARGB && targetOrder == MCO_BGRA)
            ||(srcOrder == MCO_BGRA && targetOrder == MCO_ARGB)) {
        //ARGB <==> BGRA
        desMat = argb2bgra(srcMat);
    } else if (srcOrder == MCO_ARGB && targetOrder == MCO_RGBA) {
        //ARGB ==> RGBA
        desMat = cv::Mat(srcMat.rows, srcMat.cols, srcMat.type());
        int from_to[] = {0,3, 1,0, 2,1, 3,2};
        cv::mixChannels(&srcMat, 1, &desMat, 1, from_to, 4);
    } else if (srcOrder == MCO_RGBA && targetOrder == MCO_ARGB) {
        //RGBA ==> ARGB
        desMat = cv::Mat(srcMat.rows, srcMat.cols, srcMat.type());
        int from_to[] = {0,1, 1,2, 2,3, 3,0};
        cv::mixChannels(&srcMat, 1, &desMat, 1, from_to, 4);
    } else {
        //BGRA <==> RBGA
        cv::cvtColor(srcMat, desMat, CV_BGRA2RGBA);
    }
    return desMat;
}

QImage::Format findClosestFormat(QImage::Format formatHint)
{
    QImage::Format format;
    switch (formatHint) {
    case QImage::Format_Indexed8:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
#if QT_VERSION >= 0x040400
    case QImage::Format_RGB888:
#endif
#if QT_VERSION >= 0x050200
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
#endif
#if QT_VERSION >= 0x050500
    case QImage::Format_Alpha8:
    case QImage::Format_Grayscale8:
#endif
        format = formatHint;
        break;
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        format = QImage::Format_Indexed8;
        break;
    case QImage::Format_RGB16:
        format = QImage::Format_RGB32;
        break;
#if QT_VERSION > 0x040400
    case QImage::Format_RGB444:
    case QImage::Format_RGB555:
    case QImage::Format_RGB666:
        format = QImage::Format_RGB888;
        break;
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
#endif
    default:
        format = QImage::Format_ARGB32;
        break;
    }
    return format;
}

MatColorOrder getColorOrderOfRGB32Format()
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        return MCO_BGRA;
#else
        return MCO_ARGB;
#endif
}
} //namespace


/* Convert QImage to cv::Mat
 */
cv::Mat image2Mat(const QImage &img, int requiredMatType, MatColorOrder requriedOrder)
{
    int targetDepth = CV_MAT_DEPTH(requiredMatType);
    int targetChannels = CV_MAT_CN(requiredMatType);
    Q_ASSERT(targetChannels==CV_CN_MAX || targetChannels==1 || targetChannels==3 || targetChannels==4);
    Q_ASSERT(targetDepth==CV_8U || targetDepth==CV_16U || targetDepth==CV_32F);

    if (img.isNull())
        return cv::Mat();

    //Find the closest image format that can be used in image2Mat_shared()
    QImage::Format format = findClosestFormat(img.format());
    QImage image = (format==img.format()) ? img : img.convertToFormat(format);

    MatColorOrder srcOrder;
    cv::Mat mat0 = image2Mat_shared(image, &srcOrder);

    //Adjust mat channells if needed.
    cv::Mat mat_adjustCn;
    const float maxAlpha = targetDepth==CV_8U ? 255 : (targetDepth==CV_16U ? 65535 : 1.0);
    if (targetChannels == CV_CN_MAX)
        targetChannels = mat0.channels();
    switch(targetChannels) {
    case 1:
        if (mat0.channels() == 3) {
            cv::cvtColor(mat0, mat_adjustCn, CV_RGB2GRAY);
        } else if (mat0.channels() == 4) {
            if (srcOrder == MCO_BGRA)
                cv::cvtColor(mat0, mat_adjustCn, CV_BGRA2GRAY);
            else if (srcOrder == MCO_RGBA)
                cv::cvtColor(mat0, mat_adjustCn, CV_RGBA2GRAY);
            else//MCO_ARGB
                cv::cvtColor(argb2bgra(mat0), mat_adjustCn, CV_BGRA2GRAY);
        }
        break;
    case 3:
        if (mat0.channels() == 1) {
            cv::cvtColor(mat0, mat_adjustCn, requriedOrder == MCO_BGR ? CV_GRAY2BGR : CV_GRAY2RGB);
        } else if (mat0.channels() == 3) {
            if (requriedOrder != srcOrder)
                cv::cvtColor(mat0, mat_adjustCn, CV_RGB2BGR);
        } else if (mat0.channels() == 4) {
            if (srcOrder == MCO_ARGB) {
                mat_adjustCn = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 3));
                int ARGB2RGB[] = {1,0, 2,1, 3,2};
                int ARGB2BGR[] = {1,2, 2,1, 3,0};
                cv::mixChannels(&mat0, 1, &mat_adjustCn, 1, requriedOrder == MCO_BGR ? ARGB2BGR : ARGB2RGB, 3);
            } else if (srcOrder == MCO_BGRA) {
                cv::cvtColor(mat0, mat_adjustCn, requriedOrder == MCO_BGR ? CV_BGRA2BGR : CV_BGRA2RGB);
            } else {//RGBA
                cv::cvtColor(mat0, mat_adjustCn, requriedOrder == MCO_BGR ? CV_RGBA2BGR : CV_RGBA2RGB);
            }
        }
        break;
    case 4:
        if (mat0.channels() == 1) {
            if (requriedOrder == MCO_ARGB) {
                cv::Mat alphaMat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 1), cv::Scalar(maxAlpha));
                mat_adjustCn = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 4));
                cv::Mat in[] = {alphaMat, mat0};
                int from_to[] = {0,0, 1,1, 1,2, 1,3};
                cv::mixChannels(in, 2, &mat_adjustCn, 1, from_to, 4);
            } else if (requriedOrder == MCO_RGBA) {
                cv::cvtColor(mat0, mat_adjustCn, CV_GRAY2RGBA);
            } else {//MCO_BGRA
                cv::cvtColor(mat0, mat_adjustCn, CV_GRAY2BGRA);
            }
        } else if (mat0.channels() == 3) {
            if (requriedOrder == MCO_ARGB) {
                cv::Mat alphaMat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 1), cv::Scalar(maxAlpha));
                mat_adjustCn = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 4));
                cv::Mat in[] = {alphaMat, mat0};
                int from_to[] = {0,0, 1,1, 2,2, 3,3};
                cv::mixChannels(in, 2, &mat_adjustCn, 1, from_to, 4);
            } else if (requriedOrder == MCO_RGBA) {
                cv::cvtColor(mat0, mat_adjustCn, CV_RGB2RGBA);
            } else {//MCO_BGRA
                cv::cvtColor(mat0, mat_adjustCn, CV_RGB2BGRA);
            }
        } else if (mat0.channels() == 4) {
            if (srcOrder != requriedOrder)
                mat_adjustCn = adjustChannelsOrder(mat0, srcOrder, requriedOrder);
        }
        break;
    default:
        break;
    }

    //Adjust depth if needed.
    if (targetDepth == CV_8U)
        return mat_adjustCn.empty() ? mat0.clone() : mat_adjustCn;

    if (mat_adjustCn.empty())
        mat_adjustCn = mat0;
    cv::Mat mat_adjustDepth;
    mat_adjustCn.convertTo(mat_adjustDepth, CV_MAKE_TYPE(targetDepth, mat_adjustCn.channels()), targetDepth == CV_16U ? 255.0 : 1/255.0);
    return mat_adjustDepth;
}

/* Convert cv::Mat to QImage
 */
QImage mat2Image(const cv::Mat &mat, MatColorOrder order, QImage::Format formatHint)
{
    Q_ASSERT(mat.channels()==1 || mat.channels()==3 || mat.channels()==4);
    Q_ASSERT(mat.depth()==CV_8U || mat.depth()==CV_16U || mat.depth()==CV_32F);

    if (mat.empty())
        return QImage();

    //Adjust mat channels if needed, and find proper QImage format.
    QImage::Format format;
    cv::Mat mat_adjustCn;
    if (mat.channels() == 1) {
        format = formatHint;
        if (formatHint != QImage::Format_Indexed8
        #if QT_VERSION >= 0x050500
                && formatHint != QImage::Format_Alpha8
                && formatHint != QImage::Format_Grayscale8
        #endif
                ) {
            format = QImage::Format_Indexed8;
        }
    } else if (mat.channels() == 3) {
#if QT_VERSION >= 0x040400
        format = QImage::Format_RGB888;
        if (order == MCO_BGR)
            cv::cvtColor(mat, mat_adjustCn, CV_BGR2RGB);
#else
        format = QImage::Format_RGB32;
        cv::Mat mat_tmp;
        cv::cvtColor(mat, mat_tmp, order == MCO_BGR ? CV_BGR2BGRA : CV_RGB2BGRA);
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        mat_adjustCn = mat_tmp;
#else
        mat_adjustCn = argb2bgra(mat_tmp);
#endif

#endif
    } else if (mat.channels() == 4) {
        //Find best format if the formatHint can not be applied.
        format = findClosestFormat(formatHint);
        if (format != QImage::Format_RGB32
                && format != QImage::Format_ARGB32
                && format != QImage::Format_ARGB32_Premultiplied
        #if QT_VERSION >= 0x050200
                && format != QImage::Format_RGBX8888
                && format != QImage::Format_RGBA8888
                && format != QImage::Format_RGBA8888_Premultiplied
        #endif
                ) {
#if QT_VERSION >= 0x050200
            format = order == MCO_RGBA ? QImage::Format_RGBA8888 : QImage::Format_ARGB32;
#else
            format = QImage::Format_ARGB32;
#endif
        }

        //Channel order requried by the target QImage
        MatColorOrder requiredOrder = getColorOrderOfRGB32Format();
#if QT_VERSION >= 0x050200
        if (formatHint == QImage::Format_RGBX8888
                || formatHint == QImage::Format_RGBA8888
                || formatHint == QImage::Format_RGBA8888_Premultiplied) {
            requiredOrder = MCO_RGBA;
        }
#endif

        if (order != requiredOrder)
            mat_adjustCn = adjustChannelsOrder(mat, order, requiredOrder);
    }

    if (mat_adjustCn.empty())
        mat_adjustCn = mat;

    //Adjust mat depth if needed.
    cv::Mat mat_adjustDepth = mat_adjustCn;
    if (mat.depth() != CV_8U)
        mat_adjustCn.convertTo(mat_adjustDepth, CV_8UC(mat_adjustCn.channels()), mat.depth() == CV_16U ? 1/255.0 : 255.0);

    //Should we convert the image to the format specified by formatHint?
    QImage image = mat2Image_shared(mat_adjustDepth, format);
    if (format == formatHint || formatHint == QImage::Format_Invalid)
        return image.copy();
    else
        return image.convertToFormat(formatHint);
}

/* Convert QImage to cv::Mat without data copy
 */
cv::Mat image2Mat_shared(const QImage &img, MatColorOrder *order)
{
    if (img.isNull())
        return cv::Mat();

    switch (img.format()) {
    case QImage::Format_Indexed8:
        break;
#if QT_VERSION >= 0x040400
    case QImage::Format_RGB888:
        if (order)
            *order = MCO_RGB;
        break;
#endif
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        if (order)
            *order = getColorOrderOfRGB32Format();
        break;
#if QT_VERSION >= 0x050200
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        if (order)
            *order = MCO_RGBA;
        break;
#endif
#if QT_VERSION >= 0x050500
    case QImage::Format_Alpha8:
    case QImage::Format_Grayscale8:
        break;
#endif
    default:
        return cv::Mat();
    }
    return cv::Mat(img.height(), img.width(), CV_8UC(img.depth()/8), (uchar*)img.bits(), img.bytesPerLine());
}

/* Convert  cv::Mat to QImage without data copy
 */
QImage mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint)
{
    Q_ASSERT(mat.type() == CV_8UC1 || mat.type() == CV_8UC3 || mat.type() == CV_8UC4);

    if (mat.empty())
        return QImage();

    //Adjust formatHint if needed.
    if (mat.type() == CV_8UC1) {
        if (formatHint != QImage::Format_Indexed8
        #if QT_VERSION >= 0x050500
                && formatHint != QImage::Format_Alpha8
                && formatHint != QImage::Format_Grayscale8
        #endif
                ) {
            formatHint = QImage::Format_Indexed8;
        }
#if QT_VERSION >= 0x040400
    } else if (mat.type() == CV_8UC3) {
        formatHint = QImage::Format_RGB888;
#endif
    } else if (mat.type() == CV_8UC4) {
        if (formatHint != QImage::Format_RGB32
                && formatHint != QImage::Format_ARGB32
                && formatHint != QImage::Format_ARGB32_Premultiplied
        #if QT_VERSION >= 0x050200
                && formatHint != QImage::Format_RGBX8888
                && formatHint != QImage::Format_RGBA8888
                && formatHint != QImage::Format_RGBA8888_Premultiplied
        #endif
                ) {
            formatHint = QImage::Format_ARGB32;
        }
    }

    QImage img(mat.data, mat.cols, mat.rows, mat.step, formatHint);

    //Should we add directly support for user-customed-colorTable?
    if (formatHint == QImage::Format_Indexed8) {
        QVector<QRgb> colorTable;
        for (int i=0; i<256; ++i)
            colorTable.append(qRgb(i,i,i));
        img.setColorTable(colorTable);
    }
    return img;
}

} //namespace QtOcv
