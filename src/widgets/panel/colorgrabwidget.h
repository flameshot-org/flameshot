#ifndef COLORGRABWIDGET_H
#define COLORGRABWIDGET_H

#include <QWidget>

class SidePanelWidget;
class OverlayMessage;
class CaptureWidget;

class ColorGrabWidget : public QWidget
{
    Q_OBJECT
public:
    ColorGrabWidget(QPixmap* p, CaptureWidget *captureWidget, QWidget* parent = nullptr);

    void startGrabbing();
    void abort();

    QColor color();

signals:
    void colorUpdated(const QColor& color);
    void colorGrabbed(const QColor& color);
    void grabAborted();

private:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void paintEvent(QPaintEvent* e);
    void showEvent(QShowEvent* event) override;

    QPoint cursorPos() const;
    QColor getColorAtPoint(const QPoint& point) const;
    void setExtraZoomActive(bool active);
    void setMagnifierActive(bool active);
    void updateWidget();
    void finalize();
    void UpdateCapturePoint(QMouseEvent* event);

    QPixmap* m_pixmap;
    CaptureWidget* m_captureWidget;
    QImage m_previewImage;
    QColor m_color;
    QPoint m_capturePoint;

    bool m_mousePressReceived;
    bool m_extraZoomActive;
    bool m_magnifierActive;
};

#endif // COLORGRABWIDGET_H
