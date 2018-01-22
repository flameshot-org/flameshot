// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

// This code is a modified version of the KDE software Spectacle
// /src/Gui/KSImageWidget.h commit cbbd6d45f6426ccbf1a82b15fdf98613ccccbbe9

#pragma once

#include <QGuiApplication>
#include <QStyleHints>
#include <QLabel>
#include <QColor>
#include <QMouseEvent>
#include <QPoint>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = 0);
    void setScreenshot(const QPixmap &pixmap);

signals:
    void dragInitiated();

protected:
    void mousePressEvent(QMouseEvent *event)   Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event)    Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event)      Q_DECL_OVERRIDE;

private:
    void setScaledPixmap();

    QGraphicsDropShadowEffect *m_DSEffect;
    QPixmap                    m_pixmap;
    QPoint                     m_dragStartPosition;

};
