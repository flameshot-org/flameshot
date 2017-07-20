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
#include "src/utils/confighandler.h"
#include "src/config/strftimechooserwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

FileNameEditor::FileNameEditor(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);
    initWidgets();
    initLayout();    
}

void FileNameEditor::initLayout() {
    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_nameEditor);
    m_layout->addWidget(m_outputLabel);

    QPushButton *openHelp = new QPushButton(tr("Open Helper"), this);
    connect(openHelp, &QPushButton::clicked, this, &FileNameEditor::openHelper);
    m_layout->addWidget(m_saveButton);
    QHBoxLayout *horizLayout = new QHBoxLayout();
    horizLayout->addWidget(m_saveButton);
    horizLayout->addWidget(openHelp);
    m_layout->addLayout(horizLayout);
}

void FileNameEditor::initWidgets() {
    m_nameHandler = new FileNameHandler(this);

    m_nameEditor = new QLineEdit(this);
    m_nameEditor->setMaxLength(FileNameHandler::MAX_CHARACTERS);

    m_outputLabel = new QLineEdit(this);
    m_outputLabel->setReadOnly(true);
    m_outputLabel->setFocusPolicy(Qt::NoFocus);
    QPalette pal = m_outputLabel->palette();
    QColor color = pal.color(QPalette::Disabled, m_outputLabel->backgroundRole());
    pal.setColor(QPalette::Active, m_outputLabel->backgroundRole(), color);
    m_outputLabel->setPalette(pal);

    connect(m_nameEditor, &QLineEdit::textChanged, this,
            &FileNameEditor::showParsedPattern);
    m_nameEditor->setText(ConfigHandler().getFilenamePattern());
    m_outputLabel->setText(m_nameHandler->getParsedPattern());

    m_saveButton = new QPushButton(tr("Save"), this);
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

void FileNameEditor::addToNameEditor(QString s) {
    m_nameEditor->setText(m_nameEditor->text() + s);
}

void FileNameEditor::openHelper() {
    if (!m_buttonHelper) {
        m_buttonHelper = new StrftimeChooserWidget();
        m_buttonHelper.data()->show();
        connect(this, &FileNameEditor::destroyed,
                m_buttonHelper, &StrftimeChooserWidget::deleteLater);
        connect(m_buttonHelper, &StrftimeChooserWidget::variableEmitted,
                this, &FileNameEditor::addToNameEditor);
    }
}
