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

#ifndef FILENAMEEDITOR_H
#define FILENAMEEDITOR_H

#include <QGroupBox>
#include <QPointer>

class QVBoxLayout;
class QLineEdit;
class FileNameHandler;
class QPushButton;
class StrftimeChooserWidget;

class FileNameEditor : public QGroupBox
{
    Q_OBJECT
public:
    explicit FileNameEditor(QWidget *parent = nullptr);

private:
    QVBoxLayout *m_layout;
    QLineEdit *m_outputLabel;
    QLineEdit *m_nameEditor;
    FileNameHandler *m_nameHandler;
    StrftimeChooserWidget *m_helperButtons;
    QPushButton *m_saveButton;
    QPushButton *m_resetButton;
    QPushButton *m_clearButton;

    void initLayout();
    void initWidgets();

public slots:
    void addToNameEditor(QString s);

private slots:
    void savePattern();
    void showParsedPattern(const QString &);
    void resetName();
};

#endif // FILENAMEEDITOR_H
