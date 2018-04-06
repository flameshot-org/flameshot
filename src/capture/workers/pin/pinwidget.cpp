#include "pinwidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QApplication>
#include <QClipboard>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QList>
#include <QUrl>

struct PinWidgetPrivate {
    PinWidgetPrivate(QWidget *contentWidget) : contentWidget(contentWidget) {}
    QWidget *contentWidget;
};

PinWidget::PinWidget(QWidget *contentWidget, QWidget * parent) : QWidget(parent){
    m_bMove = true;
    m_bWheel= true;
    //no window border, top, no taskbar window, completely ignore the window manager
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::X11BypassWindowManagerHint);
    //the minimum window size
    setMinimumSize(50,50);
    //drag-drop
    setAcceptDrops(true);
    //set the bottom widget background transparent
    setAttribute(Qt::WA_TranslucentBackground);
    d = new PinWidgetPrivate(contentWidget);

    // add shadow effect
    auto *shadowEffect = new QGraphicsDropShadowEffect(contentWidget);
    shadowEffect->setColor(Qt::lightGray);
    shadowEffect->setBlurRadius(2 * MARGIN);
    shadowEffect->setOffset(0, 0);
    contentWidget->setGraphicsEffect(shadowEffect);

    // add an inner window to the bottom of the window
    QGridLayout *lo = new QGridLayout();
    lo->addWidget(contentWidget, 0, 0);
    //note that it coordinates with the shadow size
    lo->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    setLayout(lo);
}

PinWidget::~PinWidget(){
    delete d;
}

void PinWidget::mousePressEvent(QMouseEvent *event){

    if (event->button() == Qt::LeftButton){
       // move
       m_PointWindow = event->globalPos() - pos();
       event->accept();
     }

}

void PinWidget::mouseMoveEvent(QMouseEvent *event){

    // move
    if (m_bMove){
        move(event->globalPos() - m_PointWindow);
    }

    event->accept();
}

void PinWidget::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event);

}

void PinWidget::wheelEvent(QWheelEvent *event){

    if (!m_bWheel) return;
    // coordinates of the current position of the mouse
    // relative to the position of the window
    float fX = 1.0 * event->pos().x() / this->width();
    float fY = 1.0 * event->pos().y() / this->height();

    int nW = 0;
    int nH = 0;
    if (event->delta() > 0){
        nW = this->width()  * 1.05;
        nH = this->height() * 1.05;
    }
    else{
        nW = this->width()  * 0.95;
        nH = this->height() * 0.95;
    }

    int x = event->globalPos().x() - (fX * nW);
    int y = event->globalPos().y() - (fY * nH);

    // window size will be scaled to (0,0), or even negative, here limited to (50,50)
    if (nW > 50 || nH > 50){
        resize(QSize(nW, nH));
        move(x, y);
    }
    event->accept();

}

void PinWidget::mouseDoubleClickEvent(QMouseEvent* event){
    Q_UNUSED(event);
    close();
}

void PinWidget::keyPressEvent(QKeyEvent *event){
    switch(event->key()){
        case Qt::Key_Up:
            move(pos().x(),pos().y()-1);
            break;
        case Qt::Key_Left:
            move(pos().x()-1,pos().y());
            break;
        case Qt::Key_Right:
            move(pos().x()+1,pos().y());
            break;
        case Qt::Key_Down:
            move(pos().x(),pos().y()+1);
            break;
    }
}

void PinWidget::dragEnterEvent(QDragEnterEvent *event){
    // simple based on the picture extension name
    if (!event->mimeData()->urls()[0].fileName().right(3).compare("jpg")
            ||!event->mimeData()->urls()[0].fileName().right(3).compare("png")
            ||!event->mimeData()->urls()[0].fileName().right(4).compare("jpeg")
            ||!event->mimeData()->urls()[0].fileName().right(3).compare("bmp")){

        event->acceptProposedAction();
    }else{
        event->ignore();
    }

}

void PinWidget::dropEvent(QDropEvent *event){
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;

    foreach(QUrl url, urls) {
        QString path = url.toLocalFile();
        QClipboard *cb = QApplication::clipboard();
        QImage img(path);
        if(img.isNull()) return;
        cb->setImage(img, QClipboard::Clipboard);
        resize(img.size());
    }
}
