#include "contentwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QPainter>

ContentWidget::ContentWidget(QWidget * parent) : QWidget(parent) {

}

void ContentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPixmap pix = returnClipboardPix();
    QRect rect = this->rect();

    QPainter painter(this);
    painter.drawPixmap(rect,pix);

}

QPixmap ContentWidget::returnClipboardPix() {

    QClipboard *board = QApplication::clipboard();
    QPixmap pix = board->pixmap();
    return pix;
}
