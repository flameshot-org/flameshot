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
    QPointer<CaptureTool> at(uint index);
    void clear();
    void undo();
    void redo();
    QList<QPointer<CaptureTool>> captureToolObjects();
    int pos();

private:
    QList<QPointer<CaptureTool>> m_captureToolObjects;
    QList<QPointer<CaptureTool>> m_captureToolObjectsTemp;
    int m_historyPos;
};

#endif // FLAMESHOT_CAPTURETOOLOBJECTSHISTORY_H
