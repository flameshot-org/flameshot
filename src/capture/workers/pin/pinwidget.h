#pragma once

#include <QWidget>
//the inner widget and outer widget margins
#define MARGIN 4

struct PinWidgetPrivate;

class PinWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinWidget(QWidget *contentWidget, QWidget *parent = nullptr);
    ~PinWidget();
    void EnableMove(bool bMove = true){ m_bMove = bMove; }
    void EnableWheel(bool bWheel= true){ m_bWheel= bWheel;}

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event );
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

signals:

public slots:

private:
    bool                    m_bMove;
    bool                    m_bWheel;
    QPoint					m_PointWindow;
    PinWidgetPrivate        *d;

};
