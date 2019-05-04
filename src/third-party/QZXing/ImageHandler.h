/*
 * Copyright 2011 QZXing authors
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

#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QImage>
#if QT_VERSION >= 0x050000
#include <QSet>
#include <QReadWriteLock>
#endif

class ImageHandler : public QObject
{
    Q_OBJECT
public:
    explicit ImageHandler(QObject *parent = 0);

    QImage extractQImage(QObject *imageObj,
                         int offsetX = 0, int offsetY = 0,
                         int width = 0, int height = 0);

public slots:
    void save(QObject *item, const QString &path,
              const int offsetX = 0, const int offsetY = 0,
              const int width = 0, const int height = 0);
private:
#if QT_VERSION >= 0x050000
    void imageGrabberReady();
    QSet<QObject *> pendingGrabbers;
    QReadWriteLock pendingGrabbersLocker;
#endif
};

#endif // IMAGEHANDLER_H
