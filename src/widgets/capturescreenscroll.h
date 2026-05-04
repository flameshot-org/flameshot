#ifndef CAPTURESCREENSCROLL_H
#define CAPTURESCREENSCROLL_H

#include <filesystem>
#include <opencv2/core.hpp>
#include <QObject>
#include <QPixmap>
#include <QImage>
#include <qwindowdefs.h>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace fs = std::filesystem;

class captureScreenScroll : public QObject
{
    Q_OBJECT
public:
    static constexpr int SCROLLBAR_WIDTH = 10;

    enum ScrollingCaptureStatus {
        Failed,
        PartiallySuccessful,
        Successful
    };

    explicit captureScreenScroll(QObject* parent = nullptr);
    explicit captureScreenScroll(void* display, WId id, int xOffset, int yOffset, int width, int height);

    ScrollingCaptureStatus status = Failed;
    int bestMatchCount = 0;
    int bestMatchIndex = 0;
    int bestIgnoreBottomOffset = 0;
    bool autoIgnoreBottomEdge = true;

    WId id = 0;
    int xOffset = 0;
    int yOffset = 0;
    int width = 0;
    int height = 0;

#if defined(Q_OS_LINUX)
    QPixmap captureScrollableArea();
#endif

#if defined(Q_OS_WIN)
    QImage captureWithPrintWindow(HWND hwnd);
#endif

    bool imagesEqual(const QImage&, const QImage&);
    cv::Mat cropFooter(const cv::Mat&, int);
    cv::Mat combineImages(const cv::Mat&, const cv::Mat&, bool);
    QPixmap cvMatToQPixmap(const cv::Mat&);
    cv::Mat cropHorizontal(const cv::Mat&);

private:
#if defined(Q_OS_LINUX)
    void* display = nullptr;
    QPixmap captureX11Window();
#endif
};

#endif // CAPTURESCREENSCROLL_H
