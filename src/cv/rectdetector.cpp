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

using pointMat = std::vector<std::vector<cv::Point>>;


RectDetector::RectDetector(const QPixmap &pixmap) : m_pixmap(pixmap) {

}

// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(const cv::Point &pt1,
                    const cv::Point &pt2,
                    const cv::Point &pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/*
 * hierarchy:
 * [next, previous, child, parent]
 * -1 == no
 * */
static pointMat filterContours(const pointMat &contours,
                               const std::vector<cv::Vec4i> &hierarchy)
{
    pointMat res;
    std::vector<cv::Point> approx;

    for (size_t i = 0; i < contours.size(); i++) {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        cv::approxPolyDP(cv::Mat(contours[i]), approx,
                         cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if (approx.size() == 4 &&
            fabs(contourArea(cv::Mat(approx))) > 800 &&
            isContourConvex(cv::Mat(approx)))
        {
            double maxCosine = 0;

            for (int j = 2; j < 5; j++) {
                // find the maximum cosine of the angle between joint edges
                double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                maxCosine = MAX(maxCosine, cosine);
            }

            // if cosines of all angles are small
            // (all angles are ~90 degree) then write quandrange
            // vertices to resultant sequence
            if (maxCosine < 0.3) {
                res.push_back(approx);
            }
        }
    }
    return res;
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static pointMat findSquares(const cv::Mat& image) {
    pointMat squares;

    cv::Mat gray0(image.size(), CV_8U);
    cv::Mat gray;

    pointMat contours;
    std::vector<cv::Vec4i> hierarchy;

    const int N = 5;
    // find squares in every color plane of the image
    for (int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        mixChannels(&image, 1, &gray0, 1, ch, 1);

        // apply Canny. Take the upper threshold from slider
        // and set the lower to 0 (which forces edges merging)
        Canny(gray0, gray, 0, 100, 5);

        // dilate canny output to remove potential
        // holes between edge segments
        cv::dilate(gray, gray, cv::Mat(), cv::Point(-1,-1));
        // find contours and store them all as a list
        cv::findContours(gray, contours, hierarchy,cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        pointMat &&tmpPoints = filterContours(contours, hierarchy);
        squares.reserve(squares.size() + tmpPoints.size());
        squares.insert(squares.end(), tmpPoints.begin(), tmpPoints.end());

        // try several threshold levels
        for (int l = 1; l < N; l++) {
            const int thresval = l *(255 / N);
            gray = gray0 >= thresval;

            cv::findContours(gray, contours, hierarchy,cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
            pointMat &&tmpPoints = filterContours(contours, hierarchy);
            squares.reserve(squares.size() + tmpPoints.size());
            squares.insert(squares.end(), tmpPoints.begin(), tmpPoints.end());
        }
    }
    return squares;
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
        auto v = findSquares(QtOcv::image2Mat(m_pixmap.toImage()));
        for (const std::vector<cv::Point> &p : v) {
            res.append(QRect(QPoint(p.at(0).x, p.at(0).y),
                             QPoint(p.at(2).x, p.at(2).y)).normalized());
        }
    }
    return res;
}
