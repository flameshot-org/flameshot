// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "arrowtool.h"
#include <QtMath>


ArrowTool::ArrowTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon ArrowTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "arrow-bottom-left.svg");
}
QString ArrowTool::name() const
{
    return tr("Arrow");
}

CaptureTool::Type ArrowTool::type() const
{
    return CaptureTool::TYPE_ARROW;
}

QString ArrowTool::description() const
{
    return tr("Set the Arrow as the paint tool");
}

CaptureTool* ArrowTool::copy(QObject* parent)
{
    auto* tool = new ArrowTool(parent);
    copyParams(this, tool);
    return tool;
}

void ArrowTool::copyParams(const ArrowTool* from, ArrowTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_arrowPath = this->m_arrowPath;
}

void ArrowTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    QPoint start = points().first;
    QPoint end = points().second;
    QLineF line(end , start);
    QPainterPath path;

    // 起止点线段长度
    int lineLength = line.length();

    // 随着起止点线段长度变化,动态调整箭头边长
    double arrowHeight = 0.2 * lineLength;
    if (arrowHeight < 12) {
        arrowHeight = 12;
    } else if (arrowHeight > 52) {
        arrowHeight = 52;
    }

    // 箭头内边的高度, 即下图对应的p3, p4点,
    // 固定为箭头高度的 0.75倍
    double shortHeight = arrowHeight * 0.75;
    // 固定箭头的角度为60度, 即等边三角形, 从而求出箭头的斜边的长度
    double sideLength1 = qSqrt(arrowHeight * arrowHeight * 1.25);
    double sideLength2 = qSqrt(shortHeight * shortHeight * 1.25);

    // 下图为箭头结构图解剖
    // s即为起点, e为结束点, 另外通过p1,p2,p3,p4四个坐标构成一个完整的箭头
    // 
    //            e
    //          /   \ 
    //        /       \
    //      /  3     4  \   
    //    1 -  |     |  - 2   
    //         |     | 
    //          |   | 
    //          |   | 
    //          |   | 
    //           \ / 
    //            s

    // 计算线段的角度(相对坐标系)
    double angle = qAtan2(end.y() - start.y(), end.x() - start.x());
 
    // 2个交叉点坐标
    QPoint x1(
        start.x() + (lineLength - arrowHeight) * qCos(angle),
        start.y() + (lineLength - arrowHeight) * qSin(angle)
    );

    QPoint x2(
        start.x() + (lineLength - shortHeight) * qCos(angle),
        start.y() + (lineLength - shortHeight) * qSin(angle)
    );

    // p1点
    QPoint p1(
        x1.x() + sideLength1 * qCos(angle - M_PI_2) / 2,
        x1.y() + sideLength1 * qSin(angle - M_PI_2) / 2
    );

    // p2点
    QPoint p2(
        x1.x() + sideLength1 * qCos(angle + M_PI_2) / 2,
        x1.y() + sideLength1 * qSin(angle + M_PI_2) / 2
    );

    // p3点
    QPoint p3(
        x2.x() + sideLength2 * qCos(angle - M_PI_2) / 4,
        x2.y() + sideLength2 * qSin(angle - M_PI_2) / 4
    );

    // p4点
    QPoint p4(
        x2.x() + sideLength2 * qCos(angle + M_PI_2) / 4,
        x2.y() + sideLength2 * qSin(angle + M_PI_2) / 4
    );


    // 开始画路径
    path.moveTo(start);
    path.lineTo(p3);
    path.lineTo(p1);
    path.lineTo(end);
    path.lineTo(p2);
    path.lineTo(p4);
    path.lineTo(start);// 闭合路径


    painter.fillPath(path, QBrush(color()));
}

void ArrowTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
