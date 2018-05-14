// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "rectdetector.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "cvmatandqimage.h"
#include <QGuiApplication>
#include <QScreen>

using pointMat = std::vector<std::vector<cv::Point>>;


RectDetector::RectDetector(const QPixmap &pixmap) : m_pixmap(pixmap) {

}

/*
 * hierarchy:
 * [next, previous, child, parent]
 * -1 == no
 * */
#include <algorithm>
static QVector<QRect> filterContours(const pointMat &contours,
                               const std::vector<cv::Vec4i> &hierarchy)
{
    QVector<QRect> res;
    res.reserve(contours.size());
    QSize maxSize = QGuiApplication::primaryScreen()->size() - QSize(5, 5);

    for (size_t i = 0; i < contours.size(); i++) {
        cv::Rect rect = cv::boundingRect(contours[i]);
        if(rect.width > maxSize.width() || rect.height > maxSize.width()) {
            continue;
        }
        if (rect.width > 14 && rect.height > 14) {
            res.append(QRect(rect.x, rect.y, rect.width, rect.height));
        }
    }
    // TODO use hierarchy and not a sort
    std::sort(res.begin(), res.end(), [](const QRect &r1, const QRect &r2) {
        return r1.width() * r1.height() < r2.width() * r2.height();
    });
    return res;
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static QVector<QRect> findSquares(const cv::Mat& image) {
    cv::Mat gray(image.size(), CV_8U);
    cv::Mat borders;

    std::vector<cv::Vec4i> hierarchy;

    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    Canny(gray, borders, 0, 190, 5);

    cv::dilate(borders, borders, cv::Mat(), cv::Point(-1,-1), 2);

    pointMat contours;
    cv::findContours(borders, contours, hierarchy,cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    return filterContours(contours, hierarchy);;
}

cv::Mat qimage_to_mat_cpy(QImage const &img, int format)
{
    return cv::Mat(img.height(), img.width(), format,
                   const_cast<uchar*>(img.bits()),
                   img.bytesPerLine()).clone();
}


QVector<QRect> RectDetector::getRects() const {
    QVector<QRect> res;
    if (!m_pixmap.isNull()) {
        res = findSquares(QtOcv::image2Mat(m_pixmap.toImage()));
    }
    return res;
}
