#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget *parent = 0);
    ~ColorPicker();
    static QVector<Qt::GlobalColor> colorList;

    QColor getDrawColor();

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

    QVector<QRect> handleMask() const;

signals:

public slots:

private:
    const int m_colorAreaSize;
    QVector<QRect> m_colorAreaList;

    QColor m_uiColor, m_drawColor;
};

#endif // COLORPICKER_H
