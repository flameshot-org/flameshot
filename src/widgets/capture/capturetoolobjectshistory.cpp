// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturetoolobjectshistory.h"

CaptureToolObjectsHistory::CaptureToolObjectsHistory(QObject* parent)
  : QObject(parent)
  , m_historyPos(-1)
{}

void CaptureToolObjectsHistory::append(QPointer<CaptureTool> captureTool)
{
    // cleanup undo history if required
    if (m_historyPos < 0 && m_captureToolObjects.size() > 0) {
        // clear all history and start from the begining
        m_captureToolObjects.clear();
    } else {
        // clear all history and start from the begining
        while (m_captureToolObjects.size() - 1 > m_historyPos &&
               m_historyPos >= 0) {
            m_captureToolObjects.removeLast();
        }
    }
    // append new item
    m_captureToolObjects.append(captureTool);
    m_historyPos = m_captureToolObjects.size() - 1;
}

QPointer<CaptureTool> CaptureToolObjectsHistory::at(uint index)
{
    if (index >= 0 && index < m_captureToolObjects.size() - 1) {
        return m_captureToolObjects.at(index);
    }
    return nullptr;
}

void CaptureToolObjectsHistory::clear()
{
    m_captureToolObjects.clear();
    m_historyPos = -1;
}

void CaptureToolObjectsHistory::undo()
{
    // circle counter
    if (m_historyPos >= 0) {
        auto captureTool = m_captureToolObjects.at(m_historyPos);
        captureTool->undo();
    }

    if (m_historyPos >= 0) {
        m_historyPos--;
    }
}

void CaptureToolObjectsHistory::redo()
{
    if (m_historyPos < m_captureToolObjects.size() - 1) {
        m_historyPos++;
    }

    // circle counter
    auto captureTool = m_captureToolObjects.at(m_historyPos);
    captureTool->redo();
}

QList<QPointer<CaptureTool>> CaptureToolObjectsHistory::captureToolObjects()
{
    m_captureToolObjectsTemp.clear();
    if (m_historyPos >= 0) {
        for (uint cnt = 0; cnt <= m_historyPos; ++cnt) {
            m_captureToolObjectsTemp.append(m_captureToolObjects.at(cnt));
        }
    }
    return m_captureToolObjectsTemp;
}

int CaptureToolObjectsHistory::pos()
{
    return m_historyPos;
}