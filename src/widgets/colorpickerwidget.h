// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include <QWidget>

class ColorPickerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPickerWidget(QWidget* parent = nullptr);

    static const QVector<QColor>& getDefaultSmallColorPalette();
    static const QVector<QColor>& getDefaultLargeColorPalette();
    void updateWidget();
    void updateSelection(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void repaint(int i, QPainter& painter);

    int m_colorAreaSize;
    int m_selectedIndex, m_lastIndex;
    QVector<QRect> m_colorAreaList;
    QVector<QColor> m_colorList;

    QColor m_uiColor;

private:
    void initColorPicker();
    static QVector<QColor> defaultSmallColorPalette, defaultLargeColorPalette;
};
