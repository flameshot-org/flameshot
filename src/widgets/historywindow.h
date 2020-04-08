// Copyright(c) 2020 Tobias Eliasson <arnestig@gmail.com>
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

#include <QMap>
#include <QWidget>
#include <QHBoxLayout>
#include <QTabWidget>

class QVBoxLayout;

class HistoryWindow : public QWidget {
    Q_OBJECT
public:
    explicit HistoryWindow(QWidget *parent = nullptr);
    void addImage( QString tabName, QPixmap p );
signals:
    void captureTaken(uint id, QPixmap p);

private slots:
    void edit();
    void editDone(uint id, QPixmap p);
    void closeTab(int index);
    void closeTabShortcut();
    void copyImage();

private:
    void initTabWidget();
    QMap< QString, QPixmap > m_historyMap;
    QVBoxLayout *m_layout;
    QTabWidget *m_tabWidget;
    QHBoxLayout *m_horizontalLayout;
};
