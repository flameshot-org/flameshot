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

#ifndef FtpConfig_H
#define FtpConfig_H

#include <QWidget>
#include <QPointer>

class QVBoxLayout;
class QLineEdit;
class FileNameHandler;
class QPushButton;
class StrftimeChooserWidget;

class FtpConfig : public QWidget
{
    Q_OBJECT
public:
    explicit FtpConfig(QWidget *parent = nullptr);

private:
    QVBoxLayout *m_layout;
    QLineEdit *m_hostname;
    QLineEdit *m_sitename;
    QLineEdit *m_port;
    QLineEdit *m_username;
    QLineEdit *m_password;

    QPushButton *m_saveButton;

    void initLayout();
    void initWidgets();

public slots:
    void updateComponents();

private slots:
    void saveSettings();

};

#endif // FtpConfig_H
