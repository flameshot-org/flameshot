// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Yurii Puchkov & Contributors

#include "capturetoolobjects.h"

#define SEARCH_RADIUS_NEAR 3
#define SEARCH_RADIUS_FAR 5
#define SEARCH_RADIUS_TEXT_HANDICAP 3

void CaptureToolObjects::append(const QPointer<CaptureTool>& captureTool)
{
    m_captureToolObjects.append(captureTool);
    m_imageCache.clear();
}

QPointer<CaptureTool> CaptureToolObjects::at(int index)
{
    if (index >= 0 && index < m_captureToolObjects.size()) {
        return m_captureToolObjects[index];
    }
    return nullptr;
}

void CaptureToolObjects::clear()
{
    m_captureToolObjects.clear();
}

QList<QPointer<CaptureTool>> CaptureToolObjects::captureToolObjects()
{
    return m_captureToolObjects;
}

int CaptureToolObjects::size()
{
    return m_captureToolObjects.size();
}

void CaptureToolObjects::removeAt(int index)
{
    if (index >= 0 && index < m_captureToolObjects.size()) {
        m_captureToolObjects.removeAt(index);
        m_imageCache.clear();
    }
}

int CaptureToolObjects::find(const QPoint& pos, const QSize& captureSize)
{
    if (m_captureToolObjects.empty()) {
        return -1;
    }
    QPixmap pixmap(captureSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    // first attempt to find at exact position
    int radius = SEARCH_RADIUS_NEAR;
    int index = findWithRadius(painter, pixmap, pos, radius);
    if (-1 == index) {
        // second attempt to find at position with radius
        radius = SEARCH_RADIUS_FAR;
        pixmap.fill(Qt::transparent);
        index = findWithRadius(painter, pixmap, pos, radius);
    }
    return index;
}

int CaptureToolObjects::findWithRadius(QPainter& painter,
                                       QPixmap& pixmap,
                                       const QPoint& pos,
                                       int radius)
{
    int index = m_captureToolObjects.size() - 1;
    bool useCache = true;
    m_imageCache.clear();
    if (m_imageCache.size() != m_captureToolObjects.size() && index >= 0) {
        // TODO - is not optimal and cache will be used just after first tool
        // object selecting
        m_imageCache.clear();
        useCache = false;
    }
    for (; index >= 0; --index) {
        QImage image;
        auto toolItem = m_captureToolObjects.at(index);
        if (useCache) {
            image = m_imageCache.at(index);
        } else {
            // create transparent image in memory and draw toolItem on it
            toolItem->drawSearchArea(painter, pixmap);

            // get color at mouse clicked position in area +/- radius
            image = pixmap.toImage();
            m_imageCache.insert(0, image);
        }

        if (toolItem->nameID() == ToolType::TEXT) {
            // Text has spaces inside to need to take a bigger radius for
            // text objects search
            radius += SEARCH_RADIUS_TEXT_HANDICAP;
        }

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

CaptureToolObjects& CaptureToolObjects::operator=(
  const CaptureToolObjects& other)
{
    // remove extra items for this if size is bigger
    while (this->m_captureToolObjects.size() >
           other.m_captureToolObjects.size()) {
        this->m_captureToolObjects.removeLast();
    }

    int count = 0;
    for (const auto& item : other.m_captureToolObjects) {
        QPointer<CaptureTool> itemCopy = item->copy(item->parent());
        if (count < this->m_captureToolObjects.size()) {
            this->m_captureToolObjects[count] = itemCopy;
        } else {
            this->m_captureToolObjects.append(itemCopy);
        }
        count++;
    }
    return *this;
}