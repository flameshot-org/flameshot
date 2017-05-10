#include "controller.h"
#include "capture/capturewidget.h"
#include "infowindow.h"
#include "configwindow.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>


Controller::Controller(QObject *parent) : QObject(parent) {
    createActions();
    createTrayIcon();
    m_trayIcon->show();

    m_nativeEventFilter = new NativeEventFilter(this);
    qApp->installNativeEventFilter(m_nativeEventFilter);
    connect(m_nativeEventFilter, &NativeEventFilter::activated, this, &Controller::slotPrintHotkey);

    m_captureWindow = nullptr;
    qApp->setQuitOnLastWindowClosed(false);
}

void Controller::createActions() {
    m_configAction = new QAction(tr("&Configuration"), this);
    connect(m_configAction, &QAction::triggered, this, &Controller::openConfigWindow);

    m_infoAction = new QAction(tr("&Information"), this);
    connect(m_infoAction, &QAction::triggered, this, &Controller::openInfoWindow);

    m_quitAction = new QAction(tr("&Quit"), this);
    connect(m_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Controller::createTrayIcon() {
    // requires a widget as parent but it should be used the whole app live period
    m_trayIconMenu = new QMenu();
    m_trayIconMenu->addAction(m_configAction);
    m_trayIconMenu->addAction(m_infoAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("Flameshot");
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon(":img/flameshot.svg"));
}

void Controller::slotPrintHotkey() {
    if (!m_captureWindow) {
        m_captureWindow = new CaptureWidget();
    }
}

void Controller::openConfigWindow() {
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
    }
}

void Controller::openInfoWindow() {
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
    }
}
