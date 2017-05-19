#include "uicoloreditor.h"
#include "color_wheel.hpp"
#include "buttonlistview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>

UIcolorEditor::UIcolorEditor(QWidget *parent) : QWidget(parent) {
    setFixedSize(200,130);
    QHBoxLayout *hLayout = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout;
    setLayout(hLayout);

    color_widgets::ColorWheel *colorWheel = new color_widgets::ColorWheel(this);
    connect(colorWheel, &color_widgets::ColorWheel::mouseReleaseOnColor, this,
            &UIcolorEditor::updateUIcolor);
    connect(colorWheel, &color_widgets::ColorWheel::colorChanged, this,
            &UIcolorEditor::updateLocalColor);

    QSettings settings;
    m_uiColor = settings.value("uiColor").value<QColor>();

    colorWheel->setColor(m_uiColor);
    colorWheel->setFixedSize(100,100);
    hLayout->addWidget(colorWheel);

    hLayout->addLayout(vLayout);



    setLayout(hLayout);
}

void UIcolorEditor::updateUIcolor() {
    QSettings settings;
    settings.setValue("uiColor", m_uiColor);
}

void UIcolorEditor::updateLocalColor(QColor c) {
    m_uiColor = c;
}
