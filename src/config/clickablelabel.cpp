#include "clickablelabel.h"


ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent) {

}

ClickableLabel::ClickableLabel(QString s, QWidget *parent) : QLabel(parent) {
    setText(s);
}

void ClickableLabel::mousePressEvent(QMouseEvent *) {
    Q_EMIT clicked();
}
