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

#include <QFrame>

class QGridLayout;
class QLabel;
class QLineEdit;
class FileNameHandler;
class QPushButton;

class FileNameEditor : public QFrame
{
    Q_OBJECT
public:
    explicit FileNameEditor(QWidget *parent = nullptr);

private:
    QGridLayout *m_layout;
    QLabel *m_outputLabel;
    QLineEdit *m_nameEditor;
    QPushButton *m_saveButton;
    FileNameHandler *m_nameHandler;

    void initLayout();
    void initWidgets();

private slots:
    void savePattern();
    void showParsedPattern(const QString &);
};

#endif // FILENAMEEDITOR_H
