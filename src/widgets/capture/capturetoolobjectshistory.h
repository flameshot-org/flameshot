// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#ifndef FLAMESHOT_CAPTURETOOLOBJECTSHISTORY_H
#define FLAMESHOT_CAPTURETOOLOBJECTSHISTORY_H

#include "src/tools/capturetool.h"
#include <QList>
#include <QPointer>

class CaptureToolObjectsHistory : public QObject
{
public:
    explicit CaptureToolObjectsHistory(QObject* parent = nullptr);
    void append(QPointer<CaptureTool> captureTool);
    QPointer<CaptureTool> at(int index);
    void clear();
    void undo();
    void redo();
    QList<QPointer<CaptureTool>> captureToolObjects();
    int historyIndex();
    void removeAt(int index);
    int size();
    int find(const QPoint& pos, const QSize& captureSize);
    QPointer<CaptureTool> toolAt(int index);

private:
    int findWithRadius(QPainter& painter,
                       QPixmap& pixmap,
                       const QPoint& pos,
                       const QSize& captureSize,
                       const int radius = 0);

    QList<QPointer<CaptureTool>> m_captureToolObjects;
    QList<QPointer<CaptureTool>> m_captureToolObjectsTemp;
    int m_historyPos;
};

#endif // FLAMESHOT_CAPTURETOOLOBJECTSHISTORY_H
