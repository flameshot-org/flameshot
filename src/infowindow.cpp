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

#include "infowindow.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>

// InfoWindow show basic information about the usage of Flameshot

InfoWindow::InfoWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(400, 275);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setWindowIcon(QIcon(":img/flameshot.svg"));
    setWindowTitle(tr("About"));

    layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("<b>Shortcuts</b>", this));
    initInfoTable();
    layout->addWidget(new QLabel("<b>License</b>", this));
    layout->addWidget(new QLabel("GPLv3+", this));

    // inform about full screen capture when no selection

    show();
}


QVector<QString> InfoWindow::keys = {
    "←↓↑→",
    "SHIFT + ←↓↑→",
    "ESC",
    "CTRL + C",
    "CTRL + S",
    "CTRL + Z",
    tr("Right Click")
};

QVector<QString> InfoWindow::description = {
    tr("Move selection 1px"),
    tr("Resize selection 1px"),
    tr("Quit capture"),
    tr("Copy to clipboard"),
    tr("Save selection as a file"),
    tr("Undo the last modification"),
    tr("Show color picker")
};


void InfoWindow::initInfoTable() {
    QTableWidget *table = new QTableWidget(this);
    layout->addWidget(table);

    table->setColumnCount(2);
    table->setRowCount(keys.size());
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->verticalHeader()->hide();
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // header creation
    QStringList names;
    names  << tr("Key") << tr("Description");
    table->setHorizontalHeaderLabels(names);
    //add content
    for (int i= 0; i < keys.size(); ++i){
        table->setItem(i, 0, new QTableWidgetItem(keys.at(i)));
        table->setItem(i, 1, new QTableWidgetItem(description.at(i)));
    }
    // adjust size
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
