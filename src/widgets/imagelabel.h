// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// This code is a modified version of the KDE software Spectacle
// /src/Gui/KSImageWidget.h commit cbbd6d45f6426ccbf1a82b15fdf98613ccccbbe9

#pragma once

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QStyleHints>

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget* parent = nullptr);
    void setScreenshot(const QPixmap& pixmap);

signals:
    void dragInitiated();

protected:
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    void setScaledPixmap();

    QGraphicsDropShadowEffect* m_DSEffect;
    QPixmap m_pixmap;
    QPoint m_dragStartPosition;
};
