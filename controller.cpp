#include "controller.h"
#include "capture/capturewidget.h"
#include "infowindow.h"
#include "configwindow.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

// Controller is the core component of Flameshot, creates the trayIcon and
// launches the capture widget

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

// creates the items of the trayIcon
void Controller::createActions() {
    m_configAction = new QAction(tr("&Configuration"), this);
    connect(m_configAction, &QAction::triggered, this, &Controller::openConfigWindow);

    m_infoAction = new QAction(tr("&Information"), this);
    connect(m_infoAction, &QAction::triggered, this, &Controller::openInfoWindow);

    m_quitAction = new QAction(tr("&Quit"), this);
    connect(m_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

// creates the trayIcon
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

// creation of a new capture
void Controller::slotPrintHotkey() {
    if (!m_captureWindow) {
        m_captureWindow = new CaptureWidget();
    }
}

// creation of the configuration window
void Controller::openConfigWindow() {
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
    }
}

// creation of the window of information
void Controller::openInfoWindow() {
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
    }
}
