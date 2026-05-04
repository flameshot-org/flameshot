#include "capturescreenscroll.h"

#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QDebug>

#if defined(Q_OS_WIN)
#include <windef.h>
#include <windows.h>
#endif

#if defined(Q_OS_LINUX)
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#endif

#include "qimage.h"
#include "qpixmap.h"

captureScreenScroll::captureScreenScroll(QObject* parent)
  : QObject{ parent }
{
}

captureScreenScroll::captureScreenScroll(void* display, WId id, int xOffset, int yOffset, int width, int height)
  : QObject(nullptr)
#if defined(Q_OS_LINUX)
  , display(display)
#endif
  , id(id)
  , xOffset(xOffset)
  , yOffset(yOffset)
  , width(width)
  , height(height)
{
}

#if defined(Q_OS_LINUX)
QPixmap captureScreenScroll::captureScrollableArea()
{
    qDebug() << "ID" << this->id
             << "xOffset" << this->xOffset
             << "yOffset" << this->yOffset
             << "width" << this->width
             << "height" << this->height;

    return captureX11Window();
}

QPixmap captureScreenScroll::captureX11Window()
{
    auto* disp = static_cast<Display*>(this->display);
    if (!disp) {
        qDebug() << "captureX11Window: invalid display";
        return QPixmap();
    }

    Window target = id == 0 ? DefaultRootWindow(disp) : static_cast<Window>(id);

    XWindowAttributes attr;
    if (!XGetWindowAttributes(disp, target, &attr)) {
        qDebug() << "captureX11Window: XGetWindowAttributes failed";
        return QPixmap();
    }

    int safeX = std::max(0, this->xOffset);
    int safeY = std::max(0, this->yOffset);
    int safeWidth = this->width;
    int safeHeight = this->height;

    if (safeX >= attr.width || safeY >= attr.height) {
        qDebug() << "captureX11Window: offsets out of bounds"
                 << "safeX" << safeX
                 << "safeY" << safeY
                 << "area" << attr.width << "x" << attr.height;
        return QPixmap();
    }

    safeWidth = std::min(safeWidth, attr.width - safeX);
    safeHeight = std::min(safeHeight, attr.height - safeY);

    if (safeWidth <= 0 || safeHeight <= 0) {
        qDebug() << "captureX11Window: invalid size"
                 << "safeWidth" << safeWidth
                 << "safeHeight" << safeHeight;
        return QPixmap();
    }

    qDebug() << "XGetImage con:"
             << "x" << safeX
             << "y" << safeY
             << "w" << safeWidth
             << "h" << safeHeight
             << "targetSize" << attr.width << "x" << attr.height
             << "targetIsRoot" << (id == 0);

    XImage* img = XGetImage(
      disp,
      target,
      safeX,
      safeY,
      static_cast<unsigned int>(safeWidth),
      static_cast<unsigned int>(safeHeight),
      AllPlanes,
      ZPixmap
      );

    if (!img) {
        qDebug() << "captureX11Window: XGetImage returned null";
        return QPixmap();
    }

    QImage qimg(
      reinterpret_cast<uchar*>(img->data),
      img->width,
      img->height,
      img->bytes_per_line,
      QImage::Format_RGB32
      );

    QPixmap pixmap = QPixmap::fromImage(qimg.copy());
    XDestroyImage(img);

    return pixmap;
}
#endif

#if defined(Q_OS_WIN)
QImage captureScreenScroll::captureWithPrintWindow(HWND hwnd)
{
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        qDebug() << "GetWindowRect failed";
        return {};
    }

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMemDC, hBitmap);

    BOOL success = PrintWindow(hwnd, hdcMemDC, PW_RENDERFULLCONTENT);

    SelectObject(hdcMemDC, hOld);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    if (!success) {
        DeleteObject(hBitmap);
        qDebug() << "PrintWindow failed";
        return {};
    }

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    QImage img(bmp.bmWidth, bmp.bmHeight, QImage::Format_ARGB32);
    GetBitmapBits(hBitmap, bmp.bmHeight * bmp.bmWidthBytes, img.bits());

    DeleteObject(hBitmap);

    return img.rgbSwapped();
}
#endif

bool captureScreenScroll::imagesEqual(const QImage& a, const QImage& b)
{
    if (a.size() != b.size()) {
        qDebug() << "imagesEqual: size mismatch:" << a.size() << b.size();
        return false;
    }

    cv::Mat imgA(a.height(), a.width(), CV_8UC4, (void*)a.bits(), a.bytesPerLine());
    cv::Mat imgB(b.height(), b.width(), CV_8UC4, (void*)b.bits(), b.bytesPerLine());

    cv::Mat diff;
    cv::absdiff(imgA, imgB, diff);
    cv::cvtColor(diff, diff, cv::COLOR_BGRA2GRAY);
    double meanDiff = cv::mean(diff)[0];

    return meanDiff < 3.0;
}

QPixmap captureScreenScroll::cvMatToQPixmap(const cv::Mat& mat)
{
    switch (mat.type())
    {
        case CV_8UC1: {
            QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
            return QPixmap::fromImage(img.copy());
        }
        case CV_8UC3: {
            QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return QPixmap::fromImage(img.rgbSwapped().copy());
        }
        case CV_8UC4: {
            QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return QPixmap::fromImage(img.copy());
        }
        default:
            throw std::runtime_error("Unsupported cv::Mat format for QPixmap conversion");
    }
}

cv::Mat captureScreenScroll::cropFooter(const cv::Mat& img, int footerHeight)
{
    if (img.rows <= footerHeight) return img.clone();
    return img.rowRange(0, img.rows - footerHeight);
}

cv::Mat captureScreenScroll::cropHorizontal(const cv::Mat& img)
{
    int scrollbar = std::min(captureScreenScroll::SCROLLBAR_WIDTH, std::max(2, img.cols / 20));
    int right = img.cols > scrollbar ? img.cols - scrollbar : img.cols;
    return img(cv::Rect(0, 0, right, img.rows)).clone();
}

cv::Mat captureScreenScroll::combineImages( const cv::Mat& result, const cv::Mat& currentImage, bool isLastImage ) {
    if ( result.empty() ) {
        status = Successful;
        return currentImage.clone();
    }

    int matchCount = 0;
    int matchIndex = 0;
    int matchLimit = currentImage.rows / 2;

    int ignoreSideOffset = std::max( 50, currentImage.cols / 20 );
    ignoreSideOffset = std::min( ignoreSideOffset, currentImage.cols / 3 );

    int compareWidth = currentImage.cols - ignoreSideOffset * 2;
    int pixelSize = result.elemSize(); // adjust dynamically

    const int ignoreBottomOffsetMax = currentImage.rows / 3;
    int ignoreBottomOffset = std::max( 50, currentImage.rows / 10 );

    if ( autoIgnoreBottomEdge ) {
        for ( int i = 0; i <= ignoreBottomOffsetMax; i++ ) {
            const uchar* r1 = result.ptr<uchar>( result.rows - 1 - i ) + ignoreSideOffset * pixelSize;
            const uchar* r2 = currentImage.ptr<uchar>( currentImage.rows - 1 - i ) + ignoreSideOffset * pixelSize;

            if ( std::memcmp( r1, r2, compareWidth * pixelSize ) != 0 ) {
                ignoreBottomOffset += i;
                break;
            }
        }

        ignoreBottomOffset = std::max( ignoreBottomOffset, bestIgnoreBottomOffset );
    }

    ignoreBottomOffset = std::min( ignoreBottomOffset, ignoreBottomOffsetMax );

    int rectBottom = result.rows - ignoreBottomOffset - 1;

    for ( int currentY = currentImage.rows - 1; currentY >= 0 && matchCount < matchLimit; --currentY ) {
        int currentMatchCount = 0;

        for ( int y = 0; currentY - y >= 0 && currentMatchCount < matchLimit; ++y ) {
            const uchar* r1 = result.ptr<uchar>( rectBottom - y ) + ignoreSideOffset * pixelSize;
            const uchar* r2 = currentImage.ptr<uchar>( currentY - y ) + ignoreSideOffset * pixelSize;

            if (std::memcmp( r1, r2, compareWidth * pixelSize ) == 0 ) {
                currentMatchCount++;
            } else {
                break;
            }
        }

        if ( currentMatchCount > matchCount ) {
            matchCount = currentMatchCount;
            matchIndex = currentY;
        }
    }

    bool bestGuess = false;

    if ( matchCount == 0 && bestMatchCount > 0 ) {
        matchCount = bestMatchCount;
        matchIndex = bestMatchIndex;
        ignoreBottomOffset = bestIgnoreBottomOffset;
        bestGuess = true;
    }

    if ( matchCount > 0 ) {
        int matchHeight = currentImage.rows - matchIndex - 1;
        if ( matchHeight > 0 ) {
            if ( matchCount > bestMatchCount ) {
                bestMatchCount = matchCount;
                bestMatchIndex = matchIndex;
                bestIgnoreBottomOffset = ignoreBottomOffset;
            }

            cv::Mat newResult( result.rows - ignoreBottomOffset + matchHeight, result.cols, result.type() );
            result.rowRange( 0, result.rows - ignoreBottomOffset).copyTo(newResult.rowRange(0, result.rows - ignoreBottomOffset ) );
            currentImage.rowRange( matchIndex + 1, currentImage.rows )
              .copyTo( newResult.rowRange( result.rows - ignoreBottomOffset, newResult.rows ) );

            if ( bestGuess )
                status = PartiallySuccessful;
            else if ( status != PartiallySuccessful )
                status = Successful;

            return newResult;
        }
    }

    status = Failed;
    return cv::Mat(); // return empty if failed
}

/*cv::Mat captureScreenScroll::combineImages(const cv::Mat& result,
                                           const cv::Mat& currentImage,
                                           bool isLastImage)
{
    if (result.empty()) {
        status = Successful;
        return currentImage.clone();
    }

    if (currentImage.empty()) {
        qDebug() << "combineImages: currentImage is empty";
        status = Failed;
        return cv::Mat();
    }

    if (result.cols != currentImage.cols) {
        qDebug() << "combineImages: width mismatch:"
                 << "result.cols =" << result.cols
                 << "currentImage.cols =" << currentImage.cols;
        status = Failed;
        return cv::Mat();
    }

    int matchCount = 0;
    int matchIndex = currentImage.rows - 1;
    const int matchLimit = currentImage.rows / 2;

    int ignoreSideOffset = std::max(50, currentImage.cols / 20);
    ignoreSideOffset = std::min(ignoreSideOffset, currentImage.cols / 3);

    const int compareWidth = currentImage.cols - ignoreSideOffset * 2;
    const int pixelSize = result.elemSize();

    if (compareWidth <= 0) {
        qDebug() << "combineImages: invalid compareWidth:"
                 << compareWidth
                 << "cols:" << currentImage.cols
                 << "ignoreSideOffset:" << ignoreSideOffset;
        status = Failed;
        return cv::Mat();
    }

    const int ignoreBottomOffsetMax = currentImage.rows / 3;
    int ignoreBottomOffset = isLastImage ? 0 : std::max(50, currentImage.rows / 10);

    if (autoIgnoreBottomEdge && !isLastImage) {
        for (int i = 0; i <= ignoreBottomOffsetMax; ++i) {
            const int r1Index = result.rows - 1 - i;
            const int r2Index = currentImage.rows - 1 - i;

            if (r1Index < 0 || r2Index < 0) {
                break;
            }

            const uchar* r1 = result.ptr<uchar>(r1Index) + ignoreSideOffset * pixelSize;
            const uchar* r2 = currentImage.ptr<uchar>(r2Index) + ignoreSideOffset * pixelSize;

            if (std::memcmp(r1, r2, compareWidth * pixelSize) != 0) {
                ignoreBottomOffset += i;
                break;
            }
        }

        ignoreBottomOffset = std::max(ignoreBottomOffset, bestIgnoreBottomOffset);
    }

    ignoreBottomOffset = std::min(ignoreBottomOffset, ignoreBottomOffsetMax);

    const int rectBottom = result.rows - ignoreBottomOffset - 1;
    if (rectBottom < 0) {
        qDebug() << "combineImages: invalid rectBottom:" << rectBottom;
        status = Failed;
        return cv::Mat();
    }

    for (int currentY = currentImage.rows - 1;
         currentY >= 0 && matchCount < matchLimit;
         --currentY) {
        int currentMatchCount = 0;

        for (int y = 0;
             currentY - y >= 0 && currentMatchCount < matchLimit;
             ++y) {
            const int resultY = rectBottom - y;
            const int currentImgY = currentY - y;

            if (resultY < 0 || currentImgY < 0) {
                break;
            }

            const uchar* r1 = result.ptr<uchar>(resultY) + ignoreSideOffset * pixelSize;
            const uchar* r2 = currentImage.ptr<uchar>(currentImgY) + ignoreSideOffset * pixelSize;

            if (std::memcmp(r1, r2, compareWidth * pixelSize) == 0) {
                currentMatchCount++;
            } else {
                break;
            }
        }

               // Prefer: 1) higher matchCount; 2) on tie, the highest match position (lowest currentY)
        if (currentMatchCount > matchCount ||
            (currentMatchCount == matchCount && currentY < matchIndex)) {
            matchCount = currentMatchCount;
            matchIndex = currentY;
        }
    }

    const int minMatchThreshold = std::max(10, currentImage.rows / 10);
    if (matchCount < minMatchThreshold) {
        qDebug() << "combineImages: match too small, skipping:"
                 << "matchCount =" << matchCount
                 << "minMatchThreshold =" << minMatchThreshold;
        status = Failed;
        return cv::Mat();
    }

    bool bestGuess = false;

    if (matchCount == 0 && bestMatchCount > 0) {
        qDebug() << "combineImages: using best previous match";
        matchCount = bestMatchCount;
        matchIndex = bestMatchIndex;
        ignoreBottomOffset = isLastImage ? 0 : bestIgnoreBottomOffset;
        bestGuess = true;
    }

    if (matchCount > 0) {
        const int appendStart = matchIndex + 1;
        const int matchHeight = currentImage.rows - appendStart;

        if (matchHeight > 0) {
            if (matchCount > bestMatchCount ||
                (matchCount == bestMatchCount && matchIndex < bestMatchIndex)) {
                bestMatchCount = matchCount;
                bestMatchIndex = matchIndex;
                bestIgnoreBottomOffset = ignoreBottomOffset;
            }

            const int topPartHeight = result.rows - ignoreBottomOffset;
            const int newRows = topPartHeight + matchHeight;

            if (topPartHeight <= 0 || newRows <= 0) {
                qDebug() << "combineImages: invalid dimensions for newResult:"
                         << "topPartHeight =" << topPartHeight
                         << "matchHeight =" << matchHeight
                         << "newRows =" << newRows;
                status = Failed;
                return cv::Mat();
            }

            cv::Mat newResult(newRows, result.cols, result.type());

            result.rowRange(0, topPartHeight)
              .copyTo(newResult.rowRange(0, topPartHeight));

            currentImage.rowRange(appendStart, currentImage.rows)
              .copyTo(newResult.rowRange(topPartHeight, newRows));

            if (bestGuess) {
                status = PartiallySuccessful;
            } else if (status != PartiallySuccessful) {
                status = Successful;
            }

            qDebug() << "combineImages: OK"
                     << "matchCount =" << matchCount
                     << "matchIndex =" << matchIndex
                     << "appendStart =" << appendStart
                     << "ignoreBottomOffset =" << ignoreBottomOffset
                     << "isLastImage =" << isLastImage
                     << "newResult =" << newResult.cols << "x" << newResult.rows;

            return newResult;
        }
    }

    qDebug() << "combineImages: no valid match found";
    status = Failed;
    return cv::Mat();
}*/
