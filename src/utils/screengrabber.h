// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/utils/desktopinfo.h"
#include <QEvent>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QScreen>

class QEventLoop;
class QWidget;

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject* parent = nullptr);
    QPixmap grabEntireDesktop(bool& ok, int preSelectedMonitor = -1);
    QRect screenGeometry(QScreen* screen);
    QPixmap grabScreen(QScreen* screenNumber, bool& ok);
    void freeDesktopPortal(bool& ok, QPixmap& res);
    QRect desktopGeometry();
    QRect logicalDesktopGeometry();
    int getSelectedMonitor() const { return m_selectedMonitor; }
    QScreen* getSelectedScreen() const;
    QPixmap selectMonitorAndCrop(const QPixmap& fullScreenshot, bool& ok);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void adjustDevicePixelRatio(QPixmap& pixmap);
    QWidget* createMonitorPreviews(const QPixmap& fullScreenshot);
    QPixmap cropToMonitor(const QPixmap& fullScreenshot, int monitorIndex);
    QPixmap windowsScreenshot(int wid);

    DesktopInfo m_info;
    QPixmap Screenshot;
    int m_selectedMonitor;
    QEventLoop* m_monitorSelectionLoop;
    bool m_userCancelled;
    static bool m_monitorSelectionActive;
};
