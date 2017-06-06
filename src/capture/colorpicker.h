// Copyright 2017 Alejandro Sirgo Rica
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget *parent = 0);
    ~ColorPicker();

    QColor getDrawColor();

    void show();
    void hide();

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

    QVector<QRect> handleMask() const;

signals:

public slots:

private:
    const int m_colorAreaSize;
    QVector<QRect> m_colorAreaList;
    static QVector<Qt::GlobalColor> colorList;

    QColor m_uiColor, m_drawColor;
};

#endif // COLORPICKER_H
