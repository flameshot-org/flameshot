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

UIcolorEditor::UIcolorEditor(QWidget *parent) : QFrame(parent) {
    setFixedSize(200,130);
    setFrameStyle(QFrame::StyledPanel);

    hLayout = new QHBoxLayout;
    vLayout = new QVBoxLayout;

    initButton();
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
    bool iconsAreWhite = QSettings().value("whiteIconColor").toBool();
//    QString bgColor = this->palette().color(QWidget::backgroundRole()).name();
//    if (bgColor < QColor(Qt::gray).name()) {
//        iconsAreWhite = true;
//    }
    m_button = new Button(Button::Type::circle, iconsAreWhite, this);
    m_button->setStyleSheet(Button::getStyle());
}
