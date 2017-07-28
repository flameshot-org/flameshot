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

#ifndef GENENERALCONF_H
#define GENENERALCONF_H

#include <QGroupBox>

class QVBoxLayout;

class GeneneralConf : public QGroupBox {
    Q_OBJECT
public:
    GeneneralConf(QWidget *parent = nullptr);

private slots:
   void showHelpChanged(bool checked);
   void showDesktopNotificationChanged(bool checked);
   void showTrayIconChanged(bool checked);

private:
    QVBoxLayout *m_layout;

    void initShowHelp();
    void initShowDesktopNotification();
    void initShowTrayIcon();
};

#endif // GENENERALCONF_H
