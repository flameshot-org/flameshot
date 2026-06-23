#include <QSystemTrayIcon>

#pragma once

class QAction;

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
    void initScreenMenu();
    void updateCaptureActionShortcut();
#if !defined(DISABLE_UPDATE_CHECKER)
    void updateCheckUpdatesMenuVisibility();
#endif

    void startGuiCapture();
    void startGuiCaptureOnScreen(int screenIndex);

    void rebuildRecentPinsMenu();

    QMenu* m_menu;
    QMenu* m_screenMenu;
    QMenu* m_recentPinsMenu;
    QAction* m_recentPinsAction;
    QAction* m_captureAction;
    QAction* m_launcherAction;
    QAction* m_pinHistoryAction;
    QAction* m_infoAction;
#if !defined(DISABLE_UPDATE_CHECKER)
    QAction* m_appUpdates;
#endif
};
