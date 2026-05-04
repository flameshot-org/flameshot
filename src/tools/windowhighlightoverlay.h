// ================= windowhighlightoverlay.h =================
#ifndef WINDOWHIGHLIGHTOVERLAY_H
#define WINDOWHIGHLIGHTOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QRect>

#if defined( Q_OS_WIN )
#include <windows.h>
#endif

class WindowHighlightOverlay : public QWidget
{
    Q_OBJECT
public:
#if defined( Q_OS_WIN )
    explicit WindowHighlightOverlay(QWidget *parent = nullptr);
    ~WindowHighlightOverlay();

    void initVirtualDesktop();            // cubre todos los monitores
    void startTracking(int intervalMs = 40); // muestra overlay + hook
    void stopTracking();                     // oculta overlay + deshook


    HWND selectedWindow() const { return selectedHwnd_; }
#endif

    static void setOverlayWindowMouseBlocking( QWidget* );
    static void blockMouseEventsOn(QWidget* );

signals:
#if defined( Q_OS_WIN )
    void panelSelected(HWND hwnd);  // Emitido al hacer clic
#endif

protected:
    void paintEvent(QPaintEvent *event) override;

private:
#if defined( Q_OS_WIN )
    QRect  getWindowUnderCursor() const;
    void   updateTarget();

    void installMouseHook();
    void uninstallMouseHook();

    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif

    static WindowHighlightOverlay* s_instance_;

    QTimer timer_;
    QRect  targetRect_;

#if defined( Q_OS_WIN )
    HHOOK  mouseHook_ { nullptr };
    HWND   selectedHwnd_ { nullptr };
#endif
};

#endif // WINDOWHIGHLIGHTOVERLAY_H
