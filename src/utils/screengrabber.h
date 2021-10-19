// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

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
    QPixmap grabScreen(QScreen* screen, bool& ok);
    QPixmap grabScreen(int screenNumber, bool& ok);
    QPixmap grabScreen(bool& ok);

private:
    DesktopInfo m_info;
};
