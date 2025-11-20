#ifndef CAPTURESCREENSCROLL_H
#define CAPTURESCREENSCROLL_H

#include <opencv2/core.hpp>
#include <QObject>
#include <qwindowdefs.h>


namespace fs = std::filesystem;

class captureScreenScroll : public QObject
{
    Q_OBJECT
public:

    static const int         SCROLLBAR_WIDTH      = 10;

    enum ScrollingCaptureStatus {
        Failed,
        PartiallySuccessful,
        Successful
    };

    ScrollingCaptureStatus status = Failed;
    int bestMatchCount = 0;
    int bestMatchIndex = 0;
    int bestIgnoreBottomOffset = 0;
    bool autoIgnoreBottomEdge = true;

    WId id;
    int xOffset;
    int yOffset;
    int width;
    int height;

    explicit captureScreenScroll( QObject* parent = nullptr );
    explicit captureScreenScroll( void *, WId , int, int, int, int );

#if defined ( Q_OS_LINUX )
    QPixmap captureScrollableArea();
#endif

#if defined ( Q_OS_WIN )
    QImage captureWithPrintWindow(HWND hwnd);
#endif

    bool imagesEqual( const QImage&, const QImage& );
    cv::Mat cropFooter( const cv::Mat&, int );
    cv::Mat combineImages( const cv::Mat&, const cv::Mat& );
    QPixmap cvMatToQPixmap( const cv::Mat & );
    cv::Mat cropHorizontal( const cv::Mat& );
    void removeDir( QString &baseDir );

signals:

private:

#if defined ( Q_OS_LINUX )
    void * display;
    QPixmap captureX11Window();
#endif


};

#endif // CAPTURESCREENSCROLL_H
