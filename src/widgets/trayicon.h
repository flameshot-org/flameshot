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
#if !defined(DISABLE_UPDATE_CHECKER)
    void enableCheckUpdatesAction(bool enable);
#endif

    void startGuiCapture();

    QMenu* m_menu;
#if !defined(DISABLE_UPDATE_CHECKER)
    QAction* m_appUpdates;
#endif
};
