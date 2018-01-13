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

#include "visualseditor.h"
#include "src/config/buttonlistview.h"
#include "src/config/uicoloreditor.h"
#include "src/utils/confighandler.h"
#include "src/config/extendedslider.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

VisualsEditor::VisualsEditor(QWidget *parent) : QWidget(parent)
{
    m_layout= new QVBoxLayout();
    setLayout(m_layout);
    initWidgets();
}

void VisualsEditor::updateComponents() {
    m_buttonList->updateComponents();
    m_colorEditor->updateComponents();
    int opacity = ConfigHandler().contrastOpacityValue();
    m_opacitySlider->setMapedValue(0, opacity, 255);
}

void VisualsEditor::initOpacitySlider() {
    m_opacitySlider = new ExtendedSlider();
    m_opacitySlider->setOrientation(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    connect(m_opacitySlider, &ExtendedSlider::modificationsEnded,
            this, &VisualsEditor::saveOpacity);
    QHBoxLayout *localLayout = new QHBoxLayout();
    localLayout->addWidget(new QLabel("0%"));
    localLayout->addWidget(m_opacitySlider);
    localLayout->addWidget(new QLabel("100%"));

    QLabel *label = new QLabel();
    QString labelMsg = tr("Opacity of area outside selection:") + " %1%";
    connect(m_opacitySlider, &ExtendedSlider::valueChanged,
            this, [labelMsg, label](int val){
        label->setText(labelMsg.arg(val));
    });
    m_layout->addWidget(label);
    m_layout->addLayout(localLayout);

    int opacity = ConfigHandler().contrastOpacityValue();
    m_opacitySlider->setMapedValue(0, opacity, 255);
}

void VisualsEditor::saveOpacity() {
    int value = m_opacitySlider->mappedValue(0, 255);
    ConfigHandler().setContrastOpacity(value);
}

void VisualsEditor::initWidgets() {
    m_colorEditor = new UIcolorEditor();
    m_layout->addWidget(m_colorEditor);

    initOpacitySlider();

    auto boxButtons = new QGroupBox();
    boxButtons->setTitle(tr("Button Selection"));
    auto listLayout = new QVBoxLayout(boxButtons);
    m_buttonList = new ButtonListView();
    m_layout->addWidget(boxButtons);
    listLayout->addWidget(m_buttonList);

    QPushButton* setAllButtons = new QPushButton(tr("Select All"));
    connect(setAllButtons, &QPushButton::clicked,
            m_buttonList, &ButtonListView::selectAll);
    listLayout->addWidget(setAllButtons);
}
