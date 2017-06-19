#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    ClickableLabel(QString s, QWidget *parent = nullptr);

signals:
    void clicked();

private:
    void mousePressEvent (QMouseEvent *) ;
};

#endif // CLICKABLELABEL_H
