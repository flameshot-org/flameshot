#ifndef COLORGRABWIDGET_H
#define COLORGRABWIDGET_H

#include <QWidget>

class SidePanelWidget;
class OverlayMessage;

class ColorGrabWidget : public QWidget
{
    Q_OBJECT
public:
    ColorGrabWidget(QPixmap* p, QWidget* parent = nullptr);

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

    QPixmap* m_pixmap;
    QImage m_previewImage;
    QColor m_color;

    bool m_mousePressReceived;
    bool m_extraZoomActive;
    bool m_magnifierActive;
};

#endif // COLORGRABWIDGET_H
