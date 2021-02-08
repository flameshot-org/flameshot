//
// Created by yuriypuchkov on 09.02.2021.
//

#ifndef FLAMESHOT_QGUIAPPCURRENTSCREEN_H
#define FLAMESHOT_QGUIAPPCURRENTSCREEN_H

#include <QPoint>

class QScreen;

class QGuiAppCurrentScreen
{
public:
    explicit QGuiAppCurrentScreen();
    QScreen* currentScreen();
    QScreen* currentScreen(const QPoint& pos);

private:
    QScreen* screenAt(const QPoint& pos);

    // class members
private:
    QScreen* m_currentScreen;
};

#endif // FLAMESHOT_QGUIAPPCURRENTSCREEN_H
