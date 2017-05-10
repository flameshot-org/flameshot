#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QPointer>
#include "nativeeventfilter.h"

class QMenu;
class QSystemTrayIcon;
class QAction;
class CaptureWidget;
class ConfigWindow;
class InfoWindow;

class Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(QObject *parent = 0);

private slots:
    void slotPrintHotkey();
    void openConfigWindow();
    void openInfoWindow();

private:
    void createActions();
    void createTrayIcon();

    QAction *m_configAction;
    QAction *m_infoAction;
    QAction *m_quitAction;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;

    NativeEventFilter *m_nativeEventFilter;

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<ConfigWindow> m_configWindow;
};

#endif // CONTROLLER_H
