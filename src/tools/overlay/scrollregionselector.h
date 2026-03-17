#ifndef SCROLLREGIONSELECTOR_H
#define SCROLLREGIONSELECTOR_H

#include <QWidget>
#include <QImage>
#include <QRect>
#include <QPoint>

class ScrollRegionSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ScrollRegionSelector(QWidget* parent = nullptr);
    void setScreenshot(const QImage& image);
    bool hasScreenshot() const;

signals:
    void selectionFinished(const QRect& rect);
    void selectionCanceled();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QRect currentRect() const;

private:
    bool m_selecting = false;
    QPoint m_start;
    QPoint m_end;
    QImage m_screenshot;
};

#endif // SCROLLREGIONSELECTOR_H
