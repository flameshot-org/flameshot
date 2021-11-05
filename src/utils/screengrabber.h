// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/utils/desktopinfo.h"
#include <QScreen>
#include <QObject>

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject* parent = nullptr);
    QPixmap grabEntireDesktop(bool& ok);
    QRect screenGeometry(QScreen* screen);
    QPixmap grabScreen(QScreen* screenNumber, bool& ok);
    void freeDesktopPortal(bool& ok, QPixmap& res);

private:
    DesktopInfo m_info;
};
