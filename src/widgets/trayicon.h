#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QImage>
#include <functional>
#include <QRect>

#if defined(Q_OS_LINUX)
#include "tools/wayland/waylandportalcapturebackend.h"
#endif

class QMenu;
class QAction;
class captureScreenScroll;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject* parent = nullptr);
    ~TrayIcon() override;

#if !defined(DISABLE_UPDATE_CHECKER)
    QAction* appUpdates();
    void enableCheckUpdatesAction(bool enable);
#endif

public slots:
    void startGuiCapture();

private:
    struct ScrollCaptureContext {
        captureScreenScroll* captureSS = nullptr;
        QString baseDir;
        int shotIdx = 0;
        QImage previousCapture;
        bool hasPrevious = false;

        std::function<QImage()> grabFrame;
        std::function<bool()> doScroll;
        std::function<void()> cleanup;
    };

#if defined(Q_OS_LINUX)
    WaylandPortalCaptureBackend* m_waylandBackend = nullptr;
#endif
private:
    void initMenu();
    void startScrollingCapture();

    QString createScrollCaptureDir() const;
    bool runCaptureLoop(ScrollCaptureContext& ctx) const;
    void waitAfterScroll(const QImage& beforeScroll) const;
    bool stitchAndSaveResult(captureScreenScroll* captureSS, const QString& baseDir) const;

#if defined(Q_OS_LINUX)
    QRect selectScrollRegionFromImage(const QImage& screenshot);
    bool setupLinuxScrollingCapture(ScrollCaptureContext& ctx);
#endif

#if defined(Q_OS_WIN)
    bool setupWindowsScrollingCapture(ScrollCaptureContext& ctx);
#endif

private:
    QMenu* m_menu = nullptr;

#if !defined(DISABLE_UPDATE_CHECKER)
    QAction* m_appUpdates = nullptr;
#endif
};

#endif // TRAYICON_H
