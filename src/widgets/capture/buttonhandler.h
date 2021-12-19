// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturetoolbutton.h"
#include <QObject>
#include <QRegion>
#include <QVector>

class CaptureToolButton;
class QRect;
class QPoint;

class ButtonHandler : public QObject
{
    Q_OBJECT
public:
    ButtonHandler(const QVector<CaptureToolButton*>&,
                  QObject* parent = nullptr);
    explicit ButtonHandler(QObject* parent = nullptr);

    void hideSectionUnderMouse(const QPoint& p);

    bool isVisible() const;
    bool buttonsAreInside() const;
    size_t size() const;

    void setButtons(const QVector<CaptureToolButton*>);
    bool contains(const QPoint& p) const;
    void updateScreenRegions(const QVector<QRect>& rects);
    void updateScreenRegions(const QRect& rect);

public slots:
    void updatePosition(const QRect& selection);
    void hide();
    void show();

private:
    QVector<QPoint> horizontalPoints(const QPoint& center,
                                     const int elements,
                                     const bool leftToRight) const;
    QVector<QPoint> verticalPoints(const QPoint& center,
                                   const int elements,
                                   const bool upToDown) const;

    int calculateShift(int elements, bool reverse) const;

    QRect intersectWithAreas(const QRect& rect);

    QVector<CaptureToolButton*> m_vectorButtons;

    QVector<QRect> m_screenRegions;

    QRect m_selection;

    int m_separator;
    int m_buttonExtendedSize;
    int m_buttonBaseSize;

    bool m_buttonsAreInside;
    bool m_blockedRight;
    bool m_blockedLeft;
    bool m_blockedBotton;
    bool m_blockedTop;
    bool m_oneHorizontalBlocked;
    bool m_horizontalyBlocked;
    bool m_allSidesBlocked;

    // aux methods
    void init();
    void resetRegionTrack();
    void updateBlockedSides();
    void expandSelection();
    void positionButtonsInside(int index);
    void ensureSelectionMinimunSize();
    void moveButtonsToPoints(const QVector<QPoint>& points, int& index);
    void adjustHorizontalCenter(QPoint& center);
};
