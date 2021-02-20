// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class ExtendedSlider;
class QVBoxLayout;
class ButtonListView;
class UIcolorEditor;

class VisualsEditor : public QWidget
{
    Q_OBJECT
public:
    explicit VisualsEditor(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private:
    QVBoxLayout* m_layout;
    ButtonListView* m_buttonList;
    UIcolorEditor* m_colorEditor;
    ExtendedSlider* m_opacitySlider;

    void initWidgets();
    void initOpacitySlider();
};
