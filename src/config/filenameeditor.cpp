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

#include "filenameeditor.h"
#include "src/utils/filenamehandler.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

FileNameEditor::FileNameEditor(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);
    initWidgets();
    initLayout();
}

void FileNameEditor::initLayout() {
    m_layout = new QGridLayout(this);
    m_layout->addWidget(m_nameEditor, 0,1);
    m_layout->addWidget(m_saveButton, 0, 0);
    m_layout->addWidget(new QLabel("Preview: ", this), 1, 0);
    m_layout->addWidget(m_outputLabel, 1, 1);
}

void FileNameEditor::initWidgets() {
    m_nameHandler = new FileNameHandler(this);

    m_nameEditor = new QLineEdit(this);
    m_nameEditor->setMaxLength(FileNameHandler::MAX_CHARACTERS);

    m_outputLabel = new QLabel(this);
    m_saveButton = new QPushButton(tr("Save"), this);

    connect(m_nameEditor, &QLineEdit::textChanged, this,
            &FileNameEditor::showParsedPattern);
    m_nameEditor->setText(m_nameHandler->getActualPattern());
    m_outputLabel->setText(m_nameHandler->getParsedPattern());

    connect(m_saveButton, &QPushButton::clicked, this, &FileNameEditor::savePattern);
}

void FileNameEditor::savePattern() {
    QString pattern = m_nameEditor->text();
    m_nameHandler->savePattern(pattern);
}

void FileNameEditor::showParsedPattern(const QString &p) {
    QString output = m_nameHandler->parseFilename(p);
    m_outputLabel->setText(output);
}
