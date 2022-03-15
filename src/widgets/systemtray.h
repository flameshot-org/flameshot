#include <QSystemTrayIcon>

#pragma once

class QAction;

class SystemTray : public QSystemTrayIcon
{
    Q_OBJECT
public:
    SystemTray(QObject* parent = nullptr);
    virtual ~SystemTray();

    QAction* appUpdates();

private:
    void initTrayIcon();
    void initMenu();
    void enableCheckUpdatesAction(bool enable);

    QMenu* m_menu;
    QAction* m_appUpdates;
};
