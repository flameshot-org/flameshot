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
        // clear all history and start from the beginning
        m_captureToolObjects.clear();
    } else {
        // clear all history and start from the beginning
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

int CaptureToolObjectsHistory::historyIndex()
{
    return m_historyPos;
}

int CaptureToolObjectsHistory::size()
{
    return m_captureToolObjects.size();
}

QPointer<CaptureTool> CaptureToolObjectsHistory::at(int index)
{
    return m_captureToolObjects[index];
}

void CaptureToolObjectsHistory::removeAt(int index)
{
    if (index >= 0 && index < m_captureToolObjects.size()) {
        m_captureToolObjects.removeAt(index);
        if (m_historyPos >= m_captureToolObjects.size()) {
            m_historyPos = m_captureToolObjects.size() - 1;
        }
    }
}

int CaptureToolObjectsHistory::find(const QPoint& pos, const QSize& captureSize)
{
    if (0 == m_captureToolObjects.size()) {
        return -1;
    }
    QPixmap pixmap(captureSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    // first attempt to find at exact position
    int index = findWithRadius(painter, pixmap, pos, captureSize);
    if (-1 == index) {
        // second attempt to find at position with radius
        int radius = 3;
        pixmap.fill(Qt::transparent);
        index = findWithRadius(painter, pixmap, pos, captureSize, radius);
    }
    return index;
}

int CaptureToolObjectsHistory::findWithRadius(QPainter& painter,
                                              QPixmap& pixmap,
                                              const QPoint& pos,
                                              const QSize& captureSize,
                                              const int radius)
{
    int index = m_captureToolObjects.size() - 1;
    for (; index >= 0; --index) {
        auto toolItem = m_captureToolObjects.at(index);

        // create transparent image in memory and draw toolItem on it
        toolItem->process(painter, pixmap, false);

        // get color at mouse clicked position in area +/- radius
        QImage image = pixmap.toImage();
        for (int x = pos.x() - radius; x <= pos.x() + radius; ++x) {
            for (int y = pos.y() - radius; y <= pos.y() + radius; ++y) {
                QRgb rgb = image.pixel(x, y);
                if (rgb != 0) {
                    // object was found, return it index (layer index)
                    return index;
                }
            }
        }
    }
    // no object at current pos found
    return -1;
}

QPointer<CaptureTool> CaptureToolObjectsHistory::toolAt(int index)
{
    if (index >= 0 && index < m_captureToolObjects.size()) {
        return m_captureToolObjects[index];
    }
    return nullptr;
}
