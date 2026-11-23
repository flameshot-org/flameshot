// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Yurii Puchkov & Contributors

#include "capturetoolobjects.h"

#include <QImage>
#include <QPainter>

#define SEARCH_RADIUS_NEAR 3
#define SEARCH_RADIUS_FAR 5
#define SEARCH_RADIUS_TEXT_HANDICAP 5

CaptureToolObjects::CaptureToolObjects(QObject* parent)
  : QObject(parent)
{}

void CaptureToolObjects::append(const QPointer<CaptureTool>& captureTool)
{
    if (!captureTool.isNull()) {
        m_captureToolObjects.append(captureTool->copy(captureTool->parent()));
    }
}

void CaptureToolObjects::insert(int index,
                                const QPointer<CaptureTool>& captureTool)
{
    if (!captureTool.isNull() && index >= 0 &&
        index <= m_captureToolObjects.size()) {
        m_captureToolObjects.insert(index,
                                    captureTool->copy(captureTool->parent()));
    }
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
    }
}

int CaptureToolObjects::find(const QPoint& pos, QSize captureSize)
{
    Q_UNUSED(captureSize)
    if (m_captureToolObjects.empty()) {
        return -1;
    }

    constexpr int maxRadius = SEARCH_RADIUS_NEAR + SEARCH_RADIUS_TEXT_HANDICAP;
    constexpr int patchDim = maxRadius * 2 + 1;
    QImage patch(patchDim, patchDim, QImage::Format_ARGB32_Premultiplied);
    QPixmap emptyPixmap;

    auto scanPatch = [&](int radius) -> bool {
        const int span = radius * 2 + 1;
        for (int y = 0; y < span; ++y) {
            const QRgb* row =
              reinterpret_cast<const QRgb*>(patch.constScanLine(y));
            for (int x = 0; x < span; ++x) {
                if (row[x] != 0) {
                    return true;
                }
            }
        }
        return false;
    };

    for (const int baseRadius : { SEARCH_RADIUS_NEAR, SEARCH_RADIUS_FAR }) {
        for (int index = m_captureToolObjects.size() - 1; index >= 0; --index) {
            auto toolItem = m_captureToolObjects.at(index);
            if (!toolItem) {
                continue;
            }
            int currentRadius = baseRadius;
            if (toolItem->type() == CaptureTool::TYPE_TEXT) {
                if (currentRadius > SEARCH_RADIUS_NEAR) {
                    continue;
                }
                currentRadius += SEARCH_RADIUS_TEXT_HANDICAP;
            }

            const QRect bRect = toolItem->boundingRect();
            const int margin = currentRadius + 8;
            if (!bRect.isNull() &&
                !bRect.adjusted(-margin, -margin, margin, margin)
                   .contains(pos)) {
                continue;
            }

            patch.fill(0);
            {
                QPainter painter(&patch);
                const int span = currentRadius * 2 + 1;
                painter.setClipRect(0, 0, span, span);
                painter.translate(-(pos.x() - currentRadius),
                                  -(pos.y() - currentRadius));
                toolItem->drawSearchArea(painter, emptyPixmap);
            }

            if (scanPatch(currentRadius)) {
                return index;
            }
        }
    }
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
