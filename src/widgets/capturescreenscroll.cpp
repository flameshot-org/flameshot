#include <opencv2/opencv.hpp>
#include <QApplication>

#include "capturescreenscroll.h"
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

#include "qimage.h"
#include "qpixmap.h"
#include "qscreen.h"

captureScreenScroll::captureScreenScroll( QObject* parent )
  : QObject{ parent }
{}

captureScreenScroll::captureScreenScroll( void *display, WId id, int xOffset, int yOffset, int width, int height )
  : display( display ),
    id( id ),
    xOffset( xOffset ),
    yOffset( yOffset ),
    width( width ),
    height( height )
{

}

QPixmap captureScreenScroll::captureScrollableArea()
{
    qDebug() << "ID "<< this -> id << "xOffset " << this -> xOffset << "yOffset " << this -> yOffset << "width " << this -> width << "height " << this -> height;

    return captureX11Window();
}

QPixmap captureScreenScroll::captureX11Window() {

    XWindowAttributes attr;
    auto *disp = static_cast<Display*>( this -> display );

    XGetWindowAttributes( disp, this -> id, &attr );

    XImage* img = XGetImage( disp, this -> id, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap );

    if ( !img ) return QPixmap();

    QImage qimg( ( uchar* ) img -> data, img -> width, img -> height, QImage::Format::Format_RGB32 );
    QPixmap pixmap = QPixmap::fromImage( qimg.copy() );  // copy to own the data
    XDestroyImage( img );

    return pixmap;
}


bool captureScreenScroll::imagesEqual( const QImage& a, const QImage& b )
{
    if ( a.size() != b.size() ) return false;
    cv::Mat imgA = cv::Mat( a.height(), a.width(), CV_8UC4, ( void* ) a.bits(), a.bytesPerLine() );
    cv::Mat imgB = cv::Mat( b.height(), b.width(), CV_8UC4, ( void* ) b.bits(), b.bytesPerLine() );

    cv::Mat diff;
    cv::absdiff( imgA, imgB, diff );
    cv::cvtColor( diff, diff, cv::COLOR_BGRA2GRAY );
    double meanDiff = cv::mean( diff )[ 0 ];

    return meanDiff < 3.0; // tolerancia ajustable
}

QPixmap captureScreenScroll::cvMatToQPixmap( const cv::Mat &mat )
{
    // Asegura que la imagen esté en formato de 8 bits por canal
    switch ( mat.type() )
    {
        case CV_8UC1: {
            QImage img( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8 );
            return QPixmap::fromImage( img.copy() );
        }
        case CV_8UC3: {
            QImage img( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888 );
            return QPixmap::fromImage( img.rgbSwapped().copy() );
        }
        case CV_8UC4: {
            QImage img( mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32 );
            return QPixmap::fromImage( img.copy() );
        }
        default:
            throw std::runtime_error( "Formato de cv::Mat no soportado para conversión a QPixmap" );
    }
}

cv::Mat captureScreenScroll::cropFooter( const cv::Mat& img, int footerHeight ) {
    if ( img.rows <= footerHeight ) return img.clone();
    return img.rowRange( 0, img.rows - footerHeight );
}

cv::Mat captureScreenScroll::cropHorizontal( const cv::Mat& img )
{
    int right = img.cols > captureScreenScroll::SCROLLBAR_WIDTH ? img.cols - captureScreenScroll::SCROLLBAR_WIDTH : img.cols;
    return img( cv::Rect( 0, 0, right, img.rows ) ).clone();
}

cv::Mat captureScreenScroll::combineImages( const cv::Mat& result, const cv::Mat& currentImage ) {
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
