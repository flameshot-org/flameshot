// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "filenameeditor.h"
#include "src/config/strftimechooserwidget.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FileNameEditor::FileNameEditor(QWidget* parent)
  : QWidget(parent)
{
    initWidgets();
    initLayout();
}

void FileNameEditor::initLayout()
{
    m_layout = new QVBoxLayout(this);
    auto* infoLabel = new QLabel(tr("Edit the name of your captures:"), this);
    infoLabel->setFixedHeight(20);
    m_layout->addWidget(infoLabel);
    m_layout->addWidget(m_helperButtons);
    m_layout->addWidget(new QLabel(tr("Edit:")));
    m_layout->addWidget(m_nameEditor);
    m_layout->addWidget(new QLabel(tr("Preview:")));
    m_layout->addWidget(m_outputLabel);

    auto* horizLayout = new QHBoxLayout();
    horizLayout->addWidget(m_saveButton);
    horizLayout->addWidget(m_resetButton);
    horizLayout->addWidget(m_clearButton);
    m_layout->addLayout(horizLayout);
}

void FileNameEditor::initWidgets()
{
    m_nameHandler = new FileNameHandler(this);

    // editor
    m_nameEditor = new QLineEdit(this);
    m_nameEditor->setMaxLength(FileNameHandler::MAX_CHARACTERS);

    // preview
    m_outputLabel = new QLineEdit(this);
    m_outputLabel->setDisabled(true);
    QString foreground = this->palette().windowText().color().name();
    m_outputLabel->setStyleSheet(QStringLiteral("color: %1").arg(foreground));
    QPalette pal = m_outputLabel->palette();
    QColor color =
      pal.color(QPalette::Disabled, m_outputLabel->backgroundRole());
    pal.setColor(QPalette::Active, m_outputLabel->backgroundRole(), color);
    m_outputLabel->setPalette(pal);

    connect(m_nameEditor,
            &QLineEdit::textChanged,
            this,
            &FileNameEditor::showParsedPattern);
    updateComponents();

    // helper buttons
    m_helperButtons = new StrftimeChooserWidget(this);
    connect(m_helperButtons,
            &StrftimeChooserWidget::variableEmitted,
            this,
            &FileNameEditor::addToNameEditor);

    // save
    m_saveButton = new QPushButton(tr("Save"), this);
    connect(
      m_saveButton, &QPushButton::clicked, this, &FileNameEditor::savePattern);
    m_saveButton->setToolTip(tr("Saves the pattern"));
    // restore previous saved values
    m_resetButton = new QPushButton(tr("Restore"), this);
    connect(
      m_resetButton, &QPushButton::clicked, this, &FileNameEditor::resetName);
    m_resetButton->setToolTip(tr("Restores the saved pattern"));
    // clear
    m_clearButton = new QPushButton(tr("Clear"), this);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_nameEditor->setText(ConfigHandler().filenamePatternDefault());
        m_nameEditor->selectAll();
        m_nameEditor->setFocus();
    });
    m_clearButton->setToolTip(tr("Deletes the name"));
}

void FileNameEditor::savePattern()
{
    QString pattern = m_nameEditor->text();
    ConfigHandler().setFilenamePattern(pattern);
}

void FileNameEditor::showParsedPattern(const QString& p)
{
    QString output = m_nameHandler->parseFilename(p);
    m_outputLabel->setText(output);
}

void FileNameEditor::resetName()
{
    m_nameEditor->setText(ConfigHandler().filenamePattern());
}

void FileNameEditor::addToNameEditor(QString s)
{
    m_nameEditor->setText(m_nameEditor->text() + s);
    m_nameEditor->setFocus();
}

void FileNameEditor::updateComponents()
{
    m_nameEditor->setText(ConfigHandler().filenamePattern());
    m_outputLabel->setText(m_nameHandler->parsedPattern());
}
