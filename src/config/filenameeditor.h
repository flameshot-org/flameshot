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

#pragma once

#include <QWidget>
#include <QPointer>

class QVBoxLayout;
class QLineEdit;
class FileNameHandler;
class QPushButton;
class StrftimeChooserWidget;

class FileNameEditor : public QWidget
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
    void updateComponents();

private slots:
    void savePattern();
    void showParsedPattern(const QString &);
    void resetName();

};
