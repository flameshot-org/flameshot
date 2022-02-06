// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "uicoloreditor.h"
#include "clickablelabel.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QMap>
#include <QSpacerItem>
#include <QVBoxLayout>

UIcolorEditor::UIcolorEditor(QWidget* parent)
  : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_hLayout = new QHBoxLayout;
    m_vLayout = new QVBoxLayout;

    const int space = QApplication::fontMetrics().lineSpacing();
    m_hLayout->addItem(new QSpacerItem(space, space, QSizePolicy::Expanding));
    m_vLayout->setAlignment(Qt::AlignVCenter);

    initButtons();
    initColorWheel();

    m_vLayout->addSpacing(space);
    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addItem(new QSpacerItem(space, space, QSizePolicy::Expanding));
    setLayout(m_hLayout);
    updateComponents();
}

void UIcolorEditor::updateComponents()
{
    ConfigHandler config;
    m_uiColor = config.uiColor();
    m_contrastColor = config.contrastUiColor();
    m_buttonContrast->setColor(m_contrastColor);
    m_buttonMainColor->setColor(m_uiColor);
    if (m_lastButtonPressed == m_buttonMainColor) {
        m_colorWheel->setColor(m_uiColor);
    } else {
        m_colorWheel->setColor(m_contrastColor);
    }
}

// updateUIcolor updates the appearance of the buttons
void UIcolorEditor::updateUIcolor()
{
    ConfigHandler config;
    if (m_lastButtonPressed == m_buttonMainColor) {
        config.setUiColor(m_uiColor);
    } else {
        config.setContrastUiColor(m_contrastColor);
    }
}

// updateLocalColor updates the local button
void UIcolorEditor::updateLocalColor(const QColor c)
{
    if (m_lastButtonPressed == m_buttonMainColor) {
        m_uiColor = c;
    } else {
        m_contrastColor = c;
    }
    m_lastButtonPressed->setColor(c);
}

void UIcolorEditor::initColorWheel()
{
    m_colorWheel = new color_widgets::ColorWheel(this);
    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorSelected,
            this,
            &UIcolorEditor::updateUIcolor);
    connect(m_colorWheel,
            &color_widgets::ColorWheel::colorChanged,
            this,
            &UIcolorEditor::updateLocalColor);

    const int size = GlobalValues::buttonBaseSize() * 3;
    m_colorWheel->setMinimumSize(size, size);
    m_colorWheel->setMaximumSize(size * 2, size * 2);
    m_colorWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_colorWheel->setToolTip(tr("Change the color moving the selectors and see"
                                " the changes in the preview buttons."));

    m_hLayout->addWidget(m_colorWheel);
}

void UIcolorEditor::initButtons()
{
    const int extraSize = GlobalValues::buttonBaseSize() / 3;
    int frameSize = GlobalValues::buttonBaseSize() + extraSize;

    m_vLayout->addWidget(new QLabel(tr("Select a Button to modify it"), this));

    auto* frame = new QGroupBox();
    frame->setFixedSize(frameSize, frameSize);

    m_buttonMainColor = new CaptureToolButton(m_buttonIconType, frame);
    m_buttonMainColor->move(m_buttonMainColor->x() + extraSize / 2,
                            m_buttonMainColor->y() + extraSize / 2);
    auto* h1 = new QHBoxLayout();
    h1->addWidget(frame);
    m_labelMain = new ClickableLabel(tr("Main Color"), this);
    h1->addWidget(m_labelMain);
    m_vLayout->addLayout(h1);

    m_buttonMainColor->setToolTip(tr("Click on this button to set the edition"
                                     " mode of the main color."));

    auto* frame2 = new QGroupBox();
    m_buttonContrast = new CaptureToolButton(m_buttonIconType, frame2);
    m_buttonContrast->move(m_buttonContrast->x() + extraSize / 2,
                           m_buttonContrast->y() + extraSize / 2);

    auto* h2 = new QHBoxLayout();
    h2->addWidget(frame2);
    frame2->setFixedSize(frameSize, frameSize);
    m_labelContrast = new ClickableLabel(tr("Contrast Color"), this);
    m_labelContrast->setStyleSheet(QStringLiteral("color : gray"));
    h2->addWidget(m_labelContrast);
    m_vLayout->addLayout(h2);

    m_buttonContrast->setToolTip(tr("Click on this button to set the edition"
                                    " mode of the contrast color."));

    connect(m_buttonMainColor,
            &CaptureToolButton::pressedButtonLeftClick,
            this,
            &UIcolorEditor::changeLastButton);
    connect(m_buttonContrast,
            &CaptureToolButton::pressedButtonLeftClick,
            this,
            &UIcolorEditor::changeLastButton);
    // clicking the labels changes the button too
    connect(m_labelMain, &ClickableLabel::clicked, this, [this] {
        changeLastButton(m_buttonMainColor);
    });
    connect(m_labelContrast, &ClickableLabel::clicked, this, [this] {
        changeLastButton(m_buttonContrast);
    });
    m_lastButtonPressed = m_buttonMainColor;
}

// visual update for the selected button
void UIcolorEditor::changeLastButton(CaptureToolButton* b)
{
    if (m_lastButtonPressed != b) {
        m_lastButtonPressed = b;

        QString offStyle(QStringLiteral("QLabel { color : gray; }"));

        if (b == m_buttonMainColor) {
            m_colorWheel->setColor(m_uiColor);
            m_labelContrast->setStyleSheet(offStyle);
            m_labelMain->setStyleSheet(styleSheet());
        } else {
            m_colorWheel->setColor(m_contrastColor);
            m_labelContrast->setStyleSheet(styleSheet());
            m_labelMain->setStyleSheet(offStyle);
        }
        b->setIcon(b->icon());
    }
}
