// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QPointer>
#include <QWidget>

class QVBoxLayout;
class QLineEdit;
class FileNameHandler;
class QPushButton;
class StrftimeChooserWidget;

class FileNameEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FileNameEditor(QWidget* parent = nullptr);

private:
    QVBoxLayout* m_layout;
    QLineEdit* m_outputLabel;
    QLineEdit* m_nameEditor;
    FileNameHandler* m_nameHandler;
    StrftimeChooserWidget* m_helperButtons;
    QPushButton* m_saveButton;
    QPushButton* m_resetButton;
    QPushButton* m_clearButton;

    void initLayout();
    void initWidgets();

public slots:
    void addToNameEditor(QString s);
    void updateComponents();

private slots:
    void savePattern();
    void showParsedPattern(const QString&);
    void resetName();
};
