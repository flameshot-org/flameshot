#include <QSystemTrayIcon>

#pragma once

class QAction;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIcon(QObject* parent = nullptr);
    virtual ~TrayIcon();

    QAction* appUpdates();

private:
    void initTrayIcon();
    void initMenu();
    void enableCheckUpdatesAction(bool enable);

    void startGuiCapture();

    QMenu* m_menu;
    QAction* m_appUpdates;
};
