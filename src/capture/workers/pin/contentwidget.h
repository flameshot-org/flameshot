#pragma once

#include <QWidget>

class ContentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContentWidget(QWidget *parent = nullptr);
    QPixmap returnClipboardPix() ;

protected:
    virtual void paintEvent(QPaintEvent *event);

signals:

public slots:

private:


};
