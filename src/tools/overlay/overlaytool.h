#ifndef OVERLAY_H
#define OVERLAY_H

#include <QtWidgets/QWidget>
#include <QTimer>
#include <QRect>
class Overlay : public QWidget {
    Q_OBJECT
public:
    explicit Overlay(QWidget* parent = nullptr);

    void updateOverlay(const QRect& rect);
    void startClickDetection();

signals:
    void windowSelected(WId id);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QRect lastRect;
    QTimer updateTimer;

    WId getRealWindowUnderCursor(WId overlayWinId);
};

#endif // OVERLAYTOOL_H
