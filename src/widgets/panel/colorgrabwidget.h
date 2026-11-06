#pragma once

#include <QWidget>

class SidePanelWidget;
class OverlayMessage;

class ColorGrabWidget : public QWidget
{
    Q_OBJECT
public:
    /// @param pixmapOrigin logical top-left of the area `p` was captured from,
    ///        needed to turn a desktop position into an offset within it.
    /// @param displayScale the display's rendering scale, which sets how much
    ///        the magnifier zooms: a denser display magnifies further.
    ColorGrabWidget(QPixmap* p,
                    const QPoint& pixmapOrigin,
                    qreal displayScale,
                    QWidget* parent = nullptr);

    void startGrabbing();

    QColor color();

signals:
    void colorUpdated(const QColor& color);
    void colorGrabbed(const QColor& color);
    void grabAborted();

private:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    void showEvent(QShowEvent* event) override;

    QPoint cursorPos() const;
    QColor getColorAtPoint(const QPoint& point) const;
    void setExtraZoomActive(bool active);
    void setMagnifierActive(bool active);
    void updateWidget();
    void finalize();

    /// Desktop position to a pixel offset within m_pixmap.
    QPoint toPixmap(const QPoint& global) const;

    QPixmap* m_pixmap;
    QPoint m_pixmapOrigin;
    qreal m_displayScale;
    QImage m_previewImage;
    QColor m_color;

    bool m_mousePressReceived;
    bool m_extraZoomActive;
    bool m_magnifierActive;
};
