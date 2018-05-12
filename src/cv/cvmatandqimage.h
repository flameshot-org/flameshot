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

#ifndef CVMATANDQIMAGE_H
#define CVMATANDQIMAGE_H

#include <QtGui/qimage.h>
#include <opencv2/core/core.hpp>

namespace QtOcv {

enum MatColorOrder {
    MCO_BGR,
    MCO_RGB,
    MCO_BGRA = MCO_BGR,
    MCO_RGBA = MCO_RGB,
    MCO_ARGB
};


/* Convert QImage to/from cv::Mat
 *
 * - cv::Mat
 *   - Supported channels
 *     - 1 channel
 *     - 3 channels (B G R), (R G B)
 *     - 4 channels (B G R A), (R G B A), (A R G B)
 *   - Supported depth
 *     - CV_8U  [0, 255]
 *     - CV_16U [0, 65535]
 *     - CV_32F [0, 1.0]
 *
 * - QImage
 *   - All of the formats of QImage are supported.
 */
cv::Mat image2Mat(const QImage &img, int requiredMatType = CV_8UC(0), MatColorOrder requiredOrder=MCO_BGR);
QImage mat2Image(const cv::Mat &mat, MatColorOrder order=MCO_BGR, QImage::Format formatHint = QImage::Format_Invalid);

/* Convert QImage to/from cv::Mat without data copy
 *
 * - Supported QImage formats and cv::Mat types are:
 *   - QImage::Format_Indexed8               <==> CV_8UC1
 *   - QImage::Format_Alpha8                 <==> CV_8UC1
 *   - QImage::Format_Grayscale8             <==> CV_8UC1
 *   - QImage::Format_RGB888                 <==> CV_8UC3 (R G B)
 *   - QImage::Format_RGB32                  <==> CV_8UC4 (A R G B or B G R A)
 *   - QImage::Format_ARGB32                 <==> CV_8UC4 (A R G B or B G R A)
 *   - QImage::Format_ARGB32_Premultiplied   <==> CV_8UC4 (A R G B or B G R A)
 *   - QImage::Format_RGBX8888               <==> CV_8UC4 (R G B A)
 *   - QImage::Format_RGBA8888               <==> CV_8UC4 (R G B A)
 *   - QImage::Format_RGBA8888_Premultiplied <==> CV_8UC4 (R G B A)
 *
 * - For QImage::Format_RGB32 ,QImage::Format_ARGB32
 *   and QImage::Format_ARGB32_Premultiplied, the
 *   color channel order of cv::Mat will be (B G R A) in little
 *   endian system or (A R G B) in big endian system.
 *
 * - User must make sure that the color channels order is the same as
 *   the color channels order requried by QImage.
 */
cv::Mat image2Mat_shared(const QImage &img, MatColorOrder *order=0);
QImage mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint = QImage::Format_Invalid);

} //namespace QtOcv

#endif // CVMATANDQIMAGE_H
