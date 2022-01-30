// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QTabWidget>
#include <QWidget>

class ExtendedSlider;
class QVBoxLayout;
class ButtonListView;
class UIcolorEditor;
class ColorPickerEditor;

class VisualsEditor : public QWidget
{
    Q_OBJECT
public:
    explicit VisualsEditor(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private:
    QVBoxLayout* m_layout;

    QTabWidget* m_tabWidget;

    UIcolorEditor* m_colorEditor;
    QWidget* m_colorEditorTab;

    ColorPickerEditor* m_colorpickerEditor;
    QWidget* m_colorpickerEditorTab;

    ButtonListView* m_buttonList;
    ExtendedSlider* m_opacitySlider;

    void initWidgets();
    void initOpacitySlider();
};
