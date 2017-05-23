// Copyright 2017 Alejandro Sirgo Rica
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

#include "uicoloreditor.h"
#include "color_wheel.hpp"
#include "src/capture/button.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QComboBox>

UIcolorEditor::UIcolorEditor(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);

    hLayout = new QHBoxLayout;
    vLayout = new QVBoxLayout;

    initButton();
    initComboBox();
    initColorWheel();
    hLayout->addLayout(vLayout);
    setLayout(hLayout);

}

void UIcolorEditor::updateUIcolor() {
    QSettings settings;
    settings.setValue("uiColor", m_uiColor);
}

void UIcolorEditor::updateLocalColor(const QColor c) {
    m_uiColor = c;
    QString style = Button::getStyle(c);
    m_button->setStyleSheet(style);
}

void UIcolorEditor::updateButtonIcon(const QString &text) {
    bool iconsAreWhite = true;
    if (text.contains("Black")) {
        iconsAreWhite = false;
    }
    m_button->setIcon(Button::getIcon(m_button->getButtonType(), iconsAreWhite));
     QSettings().setValue("whiteIconColor", iconsAreWhite);
}

void UIcolorEditor::initColorWheel() {
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
}

void UIcolorEditor::initButton() {
    QFrame *frame = new QFrame(this);
    const int extraSize = 10;
    int frameSize = Button::getButtonBaseSize() + extraSize;
    frame->setFixedSize(frameSize, frameSize);
    frame->setFrameStyle(QFrame::StyledPanel);

    bool iconsAreWhite = QSettings().value("whiteIconColor").toBool();
    m_button = new Button(Button::Type::circle, iconsAreWhite, frame);
    m_button->move(m_button->x() + extraSize/2, m_button->y() + extraSize/2);
    vLayout->addWidget(frame);

//    QString bgColor = this->palette().color(QWidget::backgroundRole()).name();
//    if (bgColor < QColor(Qt::gray).name()) {
//        iconsAreWhite = true;
    //    }
}

void UIcolorEditor::initComboBox() {
    QComboBox *comboBox = new QComboBox(this);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    comboBox->setFixedSize(comboBox->width(), comboBox->height());

    vLayout->addWidget(comboBox);

    QString textWhite = "White Icon";
    QString textBlack = "Black Icon";

    comboBox->addItem(textWhite);
    comboBox->addItem(textBlack);
    bool iconsAreWhite = QSettings().value("whiteIconColor").toBool();
    if (iconsAreWhite) {
        comboBox->setCurrentText(textWhite);
    } else {
        comboBox->setCurrentText(textBlack);
    }
    connect(comboBox, &QComboBox::currentTextChanged, this, &UIcolorEditor::updateButtonIcon);

}
