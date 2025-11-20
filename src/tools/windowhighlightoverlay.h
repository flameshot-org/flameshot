// ================= windowhighlightoverlay.h =================
#ifndef WINDOWHIGHLIGHTOVERLAY_H
#define WINDOWHIGHLIGHTOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QRect>
#include <windows.h>

class WindowHighlightOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit WindowHighlightOverlay(QWidget *parent = nullptr);
    ~WindowHighlightOverlay();

    void initVirtualDesktop();            // cubre todos los monitores
    void startTracking(int intervalMs = 40); // muestra overlay + hook
    void stopTracking();                     // oculta overlay + deshook

    HWND selectedWindow() const { return selectedHwnd_; }

    static void setOverlayWindowMouseBlocking( QWidget* );
    static void blockMouseEventsOn(QWidget* );

signals:
    void panelSelected(HWND hwnd);  // Emitido al hacer clic

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QRect  getWindowUnderCursor() const;
    void   updateTarget();

    void installMouseHook();
    void uninstallMouseHook();
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static WindowHighlightOverlay* s_instance_;

    QTimer timer_;
    QRect  targetRect_;
    HHOOK  mouseHook_ { nullptr };
    HWND   selectedHwnd_ { nullptr };
};

#endif // WINDOWHIGHLIGHTOVERLAY_H
