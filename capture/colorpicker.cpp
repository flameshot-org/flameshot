#include "colorpicker.h"
#include <QPainter>
#include <QDebug>

ColorPicker::ColorPicker(QWidget *parent) : QWidget(parent),
        m_colorAreaSize(18) {
    setMouseTracking(true);

    double radius = (colorList.size()*m_colorAreaSize/1.3)/(3.141592);
    resize(radius*2 + m_colorAreaSize, radius*2 + m_colorAreaSize);
    double degree = 360 / (colorList.size());
    double degreeAcum = degree;

    QLineF baseLine = QLineF(QPoint(radius, radius), QPoint(radius*2, radius));

    for (int i = 0; i<colorList.size(); ++i) {
        m_colorAreaList.append(QRect(baseLine.x2(), baseLine.y2(),
                                 m_colorAreaSize, m_colorAreaSize));
        baseLine.setAngle(degreeAcum);
        degreeAcum += degree;
    }
}

void ColorPicker::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QVector<QRect> rects = handleMask();
    painter.setPen(QColor(Qt::black));
    for(int i = 0; i < rects.size(); ++i) {
        painter.setBrush(QColor(colorList.at(i)));
        painter.drawRoundRect(rects.at(i), 100, 100);
    }
}

QVector<QRect> ColorPicker::handleMask() const {
    QVector<QRect> areas;
    for(QRect rect: m_colorAreaList) {
        areas.append(rect);
    }

    return areas;
}

QVector<Qt::GlobalColor> ColorPicker::colorList = {
    Qt::darkRed,
    Qt::red,
    Qt::yellow,
    Qt::green,
    Qt::darkGreen,
    Qt::darkCyan,
    Qt::blue,
    Qt::cyan,
    Qt::magenta,
    Qt::darkMagenta
};
