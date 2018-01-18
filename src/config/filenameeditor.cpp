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

#include "filenameeditor.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include "src/config/strftimechooserwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

FileNameEditor::FileNameEditor(QWidget *parent) : QWidget(parent) {
    initWidgets();
    initLayout();    
}

void FileNameEditor::initLayout() {
    m_layout = new QVBoxLayout(this);
    auto infoLabel = new QLabel(tr("Edit the name of your captures:"), this);
    infoLabel->setFixedHeight(20);
    m_layout->addWidget(infoLabel);
    m_layout->addWidget(m_helperButtons);
    m_layout->addWidget(new QLabel(tr("Edit:")));
    m_layout->addWidget(m_nameEditor);
    m_layout->addWidget(new QLabel(tr("Preview:")));
    m_layout->addWidget(m_outputLabel);

    QHBoxLayout *horizLayout = new QHBoxLayout();
    horizLayout->addWidget(m_saveButton);
    horizLayout->addWidget(m_resetButton);
    horizLayout->addWidget(m_clearButton);
    m_layout->addLayout(horizLayout);
}

void FileNameEditor::initWidgets() {
    m_nameHandler = new FileNameHandler(this);

    // editor
    m_nameEditor = new QLineEdit(this);
    m_nameEditor->setMaxLength(FileNameHandler::MAX_CHARACTERS);

    // preview
    m_outputLabel = new QLineEdit(this);
    m_outputLabel->setDisabled(true);
    QString foreground = this->palette().foreground().color().name();
    m_outputLabel->setStyleSheet(QStringLiteral("color: %1").arg(foreground));
    QPalette pal = m_outputLabel->palette();
    QColor color = pal.color(QPalette::Disabled, m_outputLabel->backgroundRole());
    pal.setColor(QPalette::Active, m_outputLabel->backgroundRole(), color);
    m_outputLabel->setPalette(pal);

    connect(m_nameEditor, &QLineEdit::textChanged, this,
            &FileNameEditor::showParsedPattern);
    updateComponents();

    // helper buttons
    m_helperButtons = new StrftimeChooserWidget(this);
    connect(m_helperButtons, &StrftimeChooserWidget::variableEmitted,
            this, &FileNameEditor::addToNameEditor);

    // save
    m_saveButton = new QPushButton(tr("Save"), this);
    connect(m_saveButton, &QPushButton::clicked, this, &FileNameEditor::savePattern);
    m_saveButton->setToolTip(tr("Saves the pattern"));
    // reset
    m_resetButton = new QPushButton(tr("Reset"), this);
    connect(m_resetButton, &QPushButton::clicked,
            this, &FileNameEditor::resetName);
    m_resetButton->setToolTip(tr("Restores the saved pattern"));
    // clear
    m_clearButton = new QPushButton(tr("Clear"), this);
    connect(m_clearButton, &QPushButton::clicked, this,
            [this](){ m_nameEditor->setText(QString());
    });
    m_clearButton->setToolTip(tr("Deletes the name"));}

void FileNameEditor::savePattern() {
    QString pattern = m_nameEditor->text();
    m_nameHandler->setPattern(pattern);
}

void FileNameEditor::showParsedPattern(const QString &p) {
    QString output = m_nameHandler->parseFilename(p);
    m_outputLabel->setText(output);
}

void FileNameEditor::resetName() {
    m_nameEditor->setText(ConfigHandler().filenamePatternValue());
}

void FileNameEditor::addToNameEditor(QString s) {
    m_nameEditor->setText(m_nameEditor->text() + s);
    m_nameEditor->setFocus();
}

void FileNameEditor::updateComponents() {
    m_nameEditor->setText(ConfigHandler().filenamePatternValue());
    m_outputLabel->setText(m_nameHandler->parsedPattern());
}
