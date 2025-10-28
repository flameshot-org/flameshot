// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// TODO: This should be removed after the complete switch to the DesktopCapture
// It is still used (but does not properly work) for non-fullscreen captures

#pragma once

#include "src/utils/desktopinfo.h"
#include <QObject>
#include <QScreen>

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject* parent = nullptr);
    QPixmap grabEntireDesktop(bool& ok);
    QRect screenGeometry(QScreen* screen);
    QPixmap grabScreen(QScreen* screenNumber, bool& ok);
    void freeDesktopPortal(bool& ok, QPixmap& res);
    void generalGrimScreenshot(bool& ok, QPixmap& res);
    QRect desktopGeometry();
    QRect logicalDesktopGeometry();

private:
    DesktopInfo m_info;
};
