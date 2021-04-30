// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Yuriy Puchkov <yuriy.puchkov@namecheap.com>

#include "qguiappcurrentscreen.h"
#include <QCursor>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QPoint>
#include <QScreen>

QGuiAppCurrentScreen::QGuiAppCurrentScreen()
{
    m_currentScreen = nullptr;
}

QScreen* QGuiAppCurrentScreen::currentScreen()
{
    return currentScreen(QCursor::pos());
}

QScreen* QGuiAppCurrentScreen::currentScreen(const QPoint& pos)
{
    m_currentScreen = screenAt(pos);
#if defined(Q_OS_MACOS)
    // On the MacOS if mouse position is at the edge of bottom or right sides
    // qGuiApp->screenAt will return nullptr, so we need to try to find current
    // screen by moving 1 pixel inside to the current desktop area
    if (!m_currentScreen && pos.x() > 0) {
        QPoint posCorrected(pos.x() - 1, pos.y());
        m_currentScreen = screenAt(posCorrected);
    }
    if (!m_currentScreen && pos.y() > 0) {
        QPoint posCorrected(pos.x(), pos.y() - 1);
        m_currentScreen = screenAt(posCorrected);
    }
    if (!m_currentScreen && pos.x() > 0 && pos.y() > 0) {
        QPoint posCorrected(pos.x() - 1, pos.y() - 1);
        m_currentScreen = screenAt(posCorrected);
    }
#endif
    if (!m_currentScreen) {
        qCritical("Unable to get current screen, starting to use primary "
                  "screen. It may be a cause of logical error and working with "
                  "a wrong screen.");
        m_currentScreen = qGuiApp->primaryScreen();
    }
    return m_currentScreen;
}

QScreen* QGuiAppCurrentScreen::screenAt(const QPoint& pos)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    m_currentScreen = qGuiApp->screenAt(pos);
#else
    for (QScreen* const screen : QGuiApplication::screens()) {
        m_currentScreen = screen;
        if (screen->geometry().contains(pos)) {
            break;
        }
    }
#endif
    return m_currentScreen;
}
