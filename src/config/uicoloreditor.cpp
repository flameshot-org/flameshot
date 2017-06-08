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
#include <QMap>
#include <QLabel>

UIcolorEditor::UIcolorEditor(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);

    hLayout = new QHBoxLayout;
    vLayout = new QVBoxLayout;

    initButtons();
    initColorWheel();
    hLayout->addLayout(vLayout);
    setLayout(hLayout);
}
// updateUIcolor updates the appearance of the buttons
void UIcolorEditor::updateUIcolor() {
    QSettings settings;
    if (m_lastButtonPressed == m_buttonMainColor) {
        settings.setValue("uiColor", m_uiColor);
    } else {
        settings.setValue("contastUiColor", m_uiColor);
    }
}
// updateLocalColor updates the local button
void UIcolorEditor::updateLocalColor(const QColor c) {
    m_uiColor = c;
    QString style = Button::getStyle(c);
    m_lastButtonPressed->setStyleSheet(style);
    updateButtonIcon();
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

    colorWheel->setToolTip(tr("Change the color moving the selectors and see"
                                " the changes in the preview buttons."));

    hLayout->addWidget(colorWheel);
}

void UIcolorEditor::initButtons() {
    const int extraSize = 10;
    int frameSize = Button::getButtonBaseSize() + extraSize;

    vLayout->addWidget(new QLabel(tr("Select a Button to modify it"), this));

    QFrame *frame = new QFrame(this);
    frame->setFixedSize(frameSize, frameSize);
    frame->setFrameStyle(QFrame::StyledPanel);


    m_buttonMainColor = new Button(Button::Type::circle, frame);
    m_buttonMainColor->move(m_buttonMainColor->x() + extraSize/2, m_buttonMainColor->y() + extraSize/2);
    QHBoxLayout *h1 = new QHBoxLayout();
    h1->addWidget(frame);
    h1->addWidget(new QLabel(tr("Main Color"), this));
    vLayout->addLayout(h1);

    m_buttonMainColor->setToolTip(tr("Click on this button to set the edition"
                                       " mode of the main color."));

    QFrame *frame2 = new QFrame(this);
    frame2->setFixedSize(frameSize, frameSize);
    frame2->setFrameStyle(QFrame::StyledPanel);

    QColor contrastColor = QSettings().value("contastUiColor").value<QColor>();
    bool whiteIconWithContast = Button::iconIsWhite(contrastColor);
    m_buttonContrast = new Button(Button::Type::circle,
                                  whiteIconWithContast, frame2);
    m_buttonContrast->setStyleSheet(Button::getStyle(contrastColor));
    m_buttonContrast->move(m_buttonContrast->x() + extraSize/2,
                           m_buttonContrast->y() + extraSize/2);

    QHBoxLayout *h2 = new QHBoxLayout();
    h2->addWidget(frame2);
    h2->addWidget(new QLabel(tr("Contrast Color"), this));
    vLayout->addLayout(h2);

    m_buttonContrast->setToolTip(tr("Click on this button to set the edition"
                                      " mode of the contrast color."));

    m_lastButtonPressed = m_buttonMainColor;
    connect(m_buttonMainColor, &Button::pressedButton,
            this, &UIcolorEditor::changeLastButton);
    connect(m_buttonContrast, &Button::pressedButton,
            this, &UIcolorEditor::changeLastButton);
}

void UIcolorEditor::updateButtonIcon() {
    m_lastButtonPressed->setIcon(Button::getIcon(m_buttonMainColor->getButtonType()));
}

void UIcolorEditor::changeLastButton(Button *b) {
    m_lastButtonPressed = b;
}
