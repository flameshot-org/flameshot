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
    layout->addWidget(new QLabel(tr("<b>Shortcuts</b>"), this));
    initInfoTable();
    layout->addWidget(new QLabel(tr("<b>License</b>"), this));
    layout->addWidget(new QLabel("GPLv3+", this));

    // inform about full screen capture when no selection

    show();
}


QVector<const char *> InfoWindow::keys = {
    "←↓↑→",
    "SHIFT + ←↓↑→",
    "ESC",
    "CTRL + C",
    "CTRL + S",
    "CTRL + Z",
    QT_TR_NOOP("Right Click")
};

QVector<const char *> InfoWindow::description = {
    QT_TR_NOOP("Move selection 1px"),
    QT_TR_NOOP("Resize selection 1px"),
    QT_TR_NOOP("Quit capture"),
    QT_TR_NOOP("Copy to clipboard"),
    QT_TR_NOOP("Save selection as a file"),
    QT_TR_NOOP("Undo the last modification"),
    QT_TR_NOOP("Show color picker")
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
        table->setItem(i, 0, new QTableWidgetItem(tr(keys.at(i))));
        table->setItem(i, 1, new QTableWidgetItem(tr(description.at(i))));
    }
    // adjust size
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
}
