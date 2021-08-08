#ifndef COLORGRABWIDGET_H
#define COLORGRABWIDGET_H

#include <QWidget>

class SidePanelWidget;

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

    void paintEvent(QPaintEvent* e);
    void hideEvent(QHideEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

    QColor getColorAtPoint(const QPoint& point);

    QPixmap* m_pixmap;
    QColor m_color;

    bool m_ignoreFirstMouseRelease = false;
};

#endif // COLORGRABWIDGET_H
