// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/widgets/colorpickerwidget.h"
#include "src/widgets/panel/colorgrabwidget.h"

class ColorPicker : public ColorPickerWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QPixmap* p, QWidget* parent = nullptr);
    void setNewColor();
signals:
    void colorSelected(QColor c);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void mouseMoveEvent(QMouseEvent* e) override;

    void startColorGrab();
    void onColorGrabFinished();

private:
    ColorGrabWidget* m_colorGrabber{};
    QPixmap* m_pixmap;
};
