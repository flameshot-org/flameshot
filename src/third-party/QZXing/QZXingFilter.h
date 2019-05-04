/*
 * Copyright 2017 QZXing authors
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

#ifndef QZXingFilter_H
#define QZXingFilter_H

#include <QObject>
#include <QAbstractVideoFilter>
#include <QDebug>
#include <QFuture>
#include <QZXing.h>

///
/// References:
///
/// https://blog.qt.io/blog/2015/03/20/introducing-video-filters-in-qt-multimedia/
/// http://doc.qt.io/qt-5/qabstractvideofilter.html
/// http://doc.qt.io/qt-5/qml-qtmultimedia-videooutput.html#filters-prop
/// http://doc.qt.io/qt-5/qvideofilterrunnable.html
/// http://doc.qt.io/qt-5/qtconcurrent-runfunction-main-cpp.html
///

/// This is used to store a QVideoFrame info while we are searching the image for QRCodes.
struct SimpleVideoFrame
{
    QByteArray data;
    QSize size;
    QVideoFrame::PixelFormat pixelFormat;

    SimpleVideoFrame()
        : size{0,0}
        , pixelFormat{QVideoFrame::Format_Invalid}
    {}

    void copyData(QVideoFrame & frame)
    {
        frame.map(QAbstractVideoBuffer::ReadOnly);

        /// Copy video frame bytes to this.data
        /// This is made to try to get a better performance (less memory allocation, faster unmap)
        /// Any other task is performed in a QFuture task, as we want to leave the UI thread asap
        if(data.size() != frame.mappedBytes())
        {
            qDebug() << "needed to resize";
            qDebug() << "size: " << data.size() << ", new size: " << frame.mappedBytes();
            data.resize(frame.mappedBytes());
        }
        memcpy(data.data(), frame.bits(), frame.mappedBytes());
        size = frame.size();
        pixelFormat = frame.pixelFormat();

        frame.unmap();
    }
};

/// Video filter is the filter that has to be registered in C++, instantiated and attached in QML
class QZXingFilter : public QAbstractVideoFilter
{
    friend class QZXingFilterRunnable;

    Q_OBJECT
        Q_PROPERTY(bool decoding READ isDecoding NOTIFY isDecodingChanged)
        Q_PROPERTY(QZXing* decoder READ getDecoder)
        Q_PROPERTY(QRectF captureRect MEMBER captureRect NOTIFY captureRectChanged)

    signals:
        void isDecodingChanged();
        void decodingFinished(bool succeeded, int decodeTime);
        void decodingStarted();
        void captureRectChanged();

    private slots:
        void handleDecodingStarted();
        void handleDecodingFinished(bool succeeded);

    private: /// Attributes
        QZXing decoder;
        bool decoding;
        QRectF captureRect;

        SimpleVideoFrame frame;
        QFuture<void> processThread;

    public:  /// Methods
        explicit QZXingFilter(QObject *parent = 0);
        virtual ~QZXingFilter();

        bool isDecoding() {return decoding; }
        QZXing* getDecoder() { return &decoder; }

        QVideoFilterRunnable * createFilterRunnable();
};

/// A new Runnable is created everytime the filter gets a new frame
class QZXingFilterRunnable : public QObject, public QVideoFilterRunnable
{
    Q_OBJECT

    public:
        explicit QZXingFilterRunnable(QZXingFilter * filter);
        virtual ~QZXingFilterRunnable();
        /// This method is called whenever we get a new frame. It runs in the UI thread.
        QVideoFrame run(QVideoFrame * input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);
        void processVideoFrameProbed(SimpleVideoFrame & videoFrame, const QRect& captureRect);

    private:
        QString decode(const QImage &image);

    private:
        QZXingFilter * filter;
};

#endif // QZXingFilter_H
