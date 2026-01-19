#include <QSystemTrayIcon>

#pragma once

class QAction;
class QActionGroup;
class QTimer;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIcon(QObject* parent = nullptr);
    virtual ~TrayIcon();

#if !defined(DISABLE_UPDATE_CHECKER)
    QAction* appUpdates();
#endif

private:
    void initTrayIcon();
    void initMenu();
    void updateCaptureActionShortcut();
#if !defined(DISABLE_UPDATE_CHECKER)
    void enableCheckUpdatesAction(bool enable);
#endif

    void startGuiCapture();
    void startGuiCaptureWithCountdown(int delayMs);
    void updateDelayActions();

    QMenu* m_menu;
    QMenu* m_delayMenu;
    QAction* m_captureAction;
    QActionGroup* m_delayActionGroup;
    QTimer* m_countdownTimer;
    int m_remainingSeconds;
#if !defined(DISABLE_UPDATE_CHECKER)
    QAction* m_appUpdates;
#endif
};
