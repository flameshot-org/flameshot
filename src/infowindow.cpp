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
#include <QKeyEvent>

// InfoWindow show basic information about the usage of Flameshot

InfoWindow::InfoWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":img/flameshot.png"));
    setWindowTitle(tr("About"));

    m_layout = new QVBoxLayout(this);
    initLabels();
    initInfoTable();
    show();
}


QVector<const char *> InfoWindow::m_keys = {
    "←↓↑→",
    "SHIFT + ←↓↑→",
    "ESC",
    "CTRL + C",
    "CTRL + S",
    "CTRL + Z",
    QT_TR_NOOP("Right Click")
};

QVector<const char *> InfoWindow::m_description = {
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
    table->setToolTip(tr("Available shorcuts in the screen capture mode."));

    m_layout->addWidget(table);

    table->setColumnCount(2);
    table->setRowCount(m_keys.size());
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->verticalHeader()->hide();
    // header creation
    QStringList names;
    names  << tr("Key") << tr("Description");
    table->setHorizontalHeaderLabels(names);
    //add content
    for (int i= 0; i < m_keys.size(); ++i){
        table->setItem(i, 0, new QTableWidgetItem(tr(m_keys.at(i))));
        table->setItem(i, 1, new QTableWidgetItem(tr(m_description.at(i))));
    }
    // adjust size
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->setMinimumWidth(400);

    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
}

void InfoWindow::initLabels() {
    m_layout->addStretch();
    QLabel *licenseTitleLabel = new QLabel(tr("<b>License</b>"), this);
    licenseTitleLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(licenseTitleLabel);
    QLabel *licenseLabel = new QLabel("GPLv3+", this);
    licenseLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(licenseLabel);
    m_layout->addStretch();

    QLabel *versionTitleLabel = new QLabel(tr("<b>Version</b>"), this);
    versionTitleLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(versionTitleLabel);
    QLabel *versionLabel = new QLabel(APP_VERSION, this);
    versionLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(versionLabel);
    m_layout->addStretch();
    m_layout->addSpacing(10);
    QLabel *shortcutsTitleLabel = new QLabel(tr("<b>Shortcuts</b>"), this);
    shortcutsTitleLabel->setAlignment(Qt::AlignHCenter);;
    m_layout->addWidget(shortcutsTitleLabel);
}

void InfoWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
            close();
    }
}
