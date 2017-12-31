#include "visualseditor.h"
#include "src/config/buttonlistview.h"
#include "src/config/uicoloreditor.h"
#include "src/utils/confighandler.h"
#include <QSlider>
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
    m_opacity = ConfigHandler().contrastOpacityValue();
    m_opacitySlider->setValue(m_opacity);
}

void VisualsEditor::initOpacitySlider() {
    QLabel *label = new QLabel(tr("Opacity of area outside selection:"));
    m_layout->addWidget(label);

    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(0, 255);
    connect(m_opacitySlider, &QSlider::sliderMoved,
            this, &VisualsEditor::updateOpacity);
    connect(m_opacitySlider, &QSlider::sliderReleased,
            this, &VisualsEditor::saveOpacity);
    QHBoxLayout *localLayout = new QHBoxLayout();
    localLayout->addWidget(new QLabel("0%"));
    localLayout->addWidget(m_opacitySlider);
    localLayout->addWidget(new QLabel("100%"));
    m_opacity = ConfigHandler().contrastOpacityValue();
    m_opacitySlider->setValue(m_opacity);
    m_layout->addLayout(localLayout);
}

void VisualsEditor::updateOpacity(int opacity) {
    m_opacity = opacity;
}

void VisualsEditor::saveOpacity() {
    ConfigHandler().setContrastOpacity(m_opacity);
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

