// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "visualseditor.h"
#include "src/config/buttonlistview.h"
#include "src/config/extendedslider.h"
#include "src/config/uicoloreditor.h"
#include "src/utils/confighandler.h"
#include <QHBoxLayout>
#include <QLabel>

VisualsEditor::VisualsEditor(QWidget* parent)
  : QWidget(parent)
{
    m_layout = new QVBoxLayout();
    setLayout(m_layout);
    initWidgets();
}

void VisualsEditor::updateComponents()
{
    m_buttonList->updateComponents();
    m_colorEditor->updateComponents();
    int opacity = ConfigHandler().contrastOpacity();
    m_opacitySlider->setMapedValue(0, opacity, 255);
}

void VisualsEditor::initOpacitySlider()
{
    m_opacitySlider = new ExtendedSlider();
    m_opacitySlider->setFocusPolicy(Qt::NoFocus);
    m_opacitySlider->setOrientation(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    QHBoxLayout* localLayout = new QHBoxLayout();
    localLayout->addWidget(new QLabel(QStringLiteral("0%")));
    localLayout->addWidget(m_opacitySlider);
    localLayout->addWidget(new QLabel(QStringLiteral("100%")));

    QLabel* label = new QLabel();
    QString labelMsg = tr("Opacity of area outside selection:") + " %1%";
    ExtendedSlider* opacitySlider = m_opacitySlider;
    connect(m_opacitySlider,
            &ExtendedSlider::valueChanged,
            this,
            [labelMsg, label, opacitySlider](int val) {
                label->setText(labelMsg.arg(val));
                ConfigHandler().setContrastOpacity(
                  opacitySlider->mappedValue(0, 255));
            });
    m_layout->addWidget(label);
    m_layout->addLayout(localLayout);

    int opacity = ConfigHandler().contrastOpacity();
    m_opacitySlider->setMapedValue(0, opacity, 255);
}

void VisualsEditor::initWidgets()
{
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
    connect(setAllButtons,
            &QPushButton::clicked,
            m_buttonList,
            &ButtonListView::selectAll);
    listLayout->addWidget(setAllButtons);
}
