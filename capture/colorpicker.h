#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget *parent = 0);
    static QVector<Qt::GlobalColor> colorList;

protected:
    void paintEvent(QPaintEvent *);
    QVector<QRect> handleMask() const;

signals:

public slots:

private:
    const int m_colorAreaSize;
    QVector<QRect> m_colorAreaList;
};

#endif // COLORPICKER_H
