// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  WhiteRectangleDetector.cpp
 *  y_wmk
 *
 *  Created by Luiz Silva on 09/02/2010.
 *  Copyright 2010 y_wmk authors All rights reserved.
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

#include <zxing/NotFoundException.h>
#include <zxing/common/detector/WhiteRectangleDetector.h>
#include <zxing/common/detector/MathUtils.h>
#include <sstream>

using std::vector;
using zxing::Ref;
using zxing::ResultPoint;
using zxing::WhiteRectangleDetector;
using zxing::common::detector::MathUtils;

// VC++
using zxing::BitMatrix;

int WhiteRectangleDetector::INIT_SIZE = 10;
int WhiteRectangleDetector::CORR = 1;

WhiteRectangleDetector::WhiteRectangleDetector(Ref<BitMatrix> image) :
   WhiteRectangleDetector(image, INIT_SIZE, image->getWidth() >> 1, image->getHeight() >> 1)
{
}

WhiteRectangleDetector::WhiteRectangleDetector(Ref<BitMatrix> image, int initSize, int x, int y) : image_(image) {
  width_ = image->getWidth();
  height_ = image->getHeight();
  
  int halfsize = initSize >> 1;
  leftInit_ = x - halfsize;
  rightInit_ = x + halfsize;
  upInit_ = y - halfsize;
  downInit_ = y + halfsize;
  
  if (upInit_ < 0 || leftInit_ < 0 || downInit_ >= height_ || rightInit_ >= width_) {
    throw NotFoundException("Invalid dimensions WhiteRectangleDetector");
  }
}

/**
 * <p>
 * Detects a candidate barcode-like rectangular region within an image. It
 * starts around the center of the image, increases the size of the candidate
 * region until it finds a white rectangular region.
 * </p>
 *
 * @return {@link vector<Ref<ResultPoint> >} describing the corners of the rectangular
 *         region. The first and last points are opposed on the diagonal, as
 *         are the second and third. The first point will be the topmost
 *         point and the last, the bottommost. The second point will be
 *         leftmost and the third, the rightmost
 * @throws NotFoundException if no Data Matrix Code can be found
*/
std::vector<Ref<ResultPoint> > WhiteRectangleDetector::detect() {
  int left = leftInit_;
  int right = rightInit_;
  int up = upInit_;
  int down = downInit_;

  bool sizeExceeded = false;
  bool aBlackPointFoundOnBorder = true;
  bool atLeastOneBlackPointFoundOnBorder = false;

  while (aBlackPointFoundOnBorder) {
    aBlackPointFoundOnBorder = false;

    // .....
    // .   |
    // .....
    bool rightBorderNotWhite = true;
    while (rightBorderNotWhite && right < width_) {
      rightBorderNotWhite = containsBlackPoint(up, down, right, false);
      if (rightBorderNotWhite) {
        right++;
        aBlackPointFoundOnBorder = true;
      }
    }

    if (right >= width_) {
      sizeExceeded = true;
      break;
    }

    // .....
    // .   .
    // .___.
    bool bottomBorderNotWhite = true;
    while (bottomBorderNotWhite && down < height_) {
      bottomBorderNotWhite = containsBlackPoint(left, right, down, true);
      if (bottomBorderNotWhite) {
        down++;
        aBlackPointFoundOnBorder = true;
      }
    }

    if (down >= height_) {
      sizeExceeded = true;
      break;
    }

    // .....
    // |   .
    // .....
    bool leftBorderNotWhite = true;
    while (leftBorderNotWhite && left >= 0) {
      leftBorderNotWhite = containsBlackPoint(up, down, left, false);
      if (leftBorderNotWhite) {
        left--;
        aBlackPointFoundOnBorder = true;
      }
    }

    if (left < 0) {
      sizeExceeded = true;
      break;
    }

    // .___.
    // .   .
    // .....
    bool topBorderNotWhite = true;
    while (topBorderNotWhite && up >= 0) {
      topBorderNotWhite = containsBlackPoint(left, right, up, true);
      if (topBorderNotWhite) {
        up--;
        aBlackPointFoundOnBorder = true;
      }
    }

    if (up < 0) {
      sizeExceeded = true;
      break;
    }

    if (aBlackPointFoundOnBorder) {
      atLeastOneBlackPointFoundOnBorder = true;
    }

  }
  if (!sizeExceeded && atLeastOneBlackPointFoundOnBorder) {

    int maxSize = right - left;

    Ref<ResultPoint> z(NULL);
    //go up right
    for (int i = 1; i < maxSize; i++) {
      z = getBlackPointOnSegment(left, down - i, left + i, down);
      if (z != NULL) {
        break;
      }
    }

    if (z == NULL) {
      throw NotFoundException("z == NULL");
    }

    Ref<ResultPoint> t(NULL);
    //go down right
    for (int i = 1; i < maxSize; i++) {
      t = getBlackPointOnSegment(left, up + i, left + i, up);
      if (t != NULL) {
        break;
      }
    }

    if (t == NULL) {
      throw NotFoundException("t == NULL");
    }

    Ref<ResultPoint> x(NULL);
    //go down left
    for (int i = 1; i < maxSize; i++) {
      x = getBlackPointOnSegment(right, up + i, right - i, up);
      if (x != NULL) {
        break;
      }
    }

    if (x == NULL) {
      throw NotFoundException("x == NULL");
    }

    Ref<ResultPoint> y(NULL);
    //go up left
    for (int i = 1; i < maxSize; i++) {
      y = getBlackPointOnSegment(right, down - i, right - i, down);
      if (y != NULL) {
        break;
      }
    }

    if (y == NULL) {
      throw NotFoundException("y == NULL");
    }

    return centerEdges(y, z, x, t);

  } else {
    throw NotFoundException("No black point found on border");
  }
}

Ref<ResultPoint>
WhiteRectangleDetector::getBlackPointOnSegment(int aX_, int aY_, int bX_, int bY_) {
  float aX = float(aX_), aY = float(aY_), bX = float(bX_), bY = float(bY_);
  int dist = MathUtils::round(MathUtils::distance(aX, aY, bX, bY));
  float xStep = (bX - aX) / dist;
  float yStep = (bY - aY) / dist;

  for (int i = 0; i < dist; i++) {
    int x = MathUtils::round(aX + i * xStep);
    int y = MathUtils::round(aY + i * yStep);
    if (image_->get(x, y)) {
      Ref<ResultPoint> point(new ResultPoint(float(x), float(y)));
      return point;
    }
  }
  Ref<ResultPoint> point(NULL);
  return point;
}

/**
 * recenters the points of a constant distance towards the center
 *
 * @param y bottom most point
 * @param z left most point
 * @param x right most point
 * @param t top most point
 * @return {@link vector<Ref<ResultPoint> >} describing the corners of the rectangular
 *         region. The first and last points are opposed on the diagonal, as
 *         are the second and third. The first point will be the topmost
 *         point and the last, the bottommost. The second point will be
 *         leftmost and the third, the rightmost
 */
vector<Ref<ResultPoint> > WhiteRectangleDetector::centerEdges(Ref<ResultPoint> y, Ref<ResultPoint> z,
                                  Ref<ResultPoint> x, Ref<ResultPoint> t) {

  //
  //       t            t
  //  z                      x
  //        x    OR    z
  //   y                    y
  //

  float yi = y->getX();
  float yj = y->getY();
  float zi = z->getX();
  float zj = z->getY();
  float xi = x->getX();
  float xj = x->getY();
  float ti = t->getX();
  float tj = t->getY();

  std::vector<Ref<ResultPoint> > corners(4);
  if (yi < (float)width_/2.0f) {
    Ref<ResultPoint> pointA(new ResultPoint(ti - CORR, tj + CORR));
    Ref<ResultPoint> pointB(new ResultPoint(zi + CORR, zj + CORR));
    Ref<ResultPoint> pointC(new ResultPoint(xi - CORR, xj - CORR));
    Ref<ResultPoint> pointD(new ResultPoint(yi + CORR, yj - CORR));
	  corners[0].reset(pointA);
	  corners[1].reset(pointB);
	  corners[2].reset(pointC);
	  corners[3].reset(pointD);
  } else {
    Ref<ResultPoint> pointA(new ResultPoint(ti + CORR, tj + CORR));
    Ref<ResultPoint> pointB(new ResultPoint(zi + CORR, zj - CORR));
    Ref<ResultPoint> pointC(new ResultPoint(xi - CORR, xj + CORR));
    Ref<ResultPoint> pointD(new ResultPoint(yi - CORR, yj - CORR));
	  corners[0].reset(pointA);
	  corners[1].reset(pointB);
	  corners[2].reset(pointC);
	  corners[3].reset(pointD);
  }
  return corners;
}

/**
 * Determines whether a segment contains a black point
 *
 * @param a          min value of the scanned coordinate
 * @param b          max value of the scanned coordinate
 * @param fixed      value of fixed coordinate
 * @param horizontal set to true if scan must be horizontal, false if vertical
 * @return true if a black point has been found, else false.
 */
bool WhiteRectangleDetector::containsBlackPoint(int a, int b, int fixed, bool horizontal) {
  if (horizontal) {
    for (int x = a; x <= b; x++) {
      if (image_->get(x, fixed)) {
        return true;
      }
    }
  } else {
    for (int y = a; y <= b; y++) {
      if (image_->get(fixed, y)) {
        return true;
      }
    }
  }

  return false;
}
