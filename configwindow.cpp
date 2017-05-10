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

#include "configwindow.h"
#include "capture/button.h"
#include <QSettings>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QLabel>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget *parent) : QWidget(parent){
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":img/flameshot.svg"));
    setWindowTitle(tr("Configuration"));

    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *buttonSelectLabel = new QLabel("Choose the buttons to enable", this);
    baseLayout->addWidget(buttonSelectLabel);

    QListWidget *buttonList = new QListWidget(this);
    baseLayout->addWidget(buttonList);

    buttonList->setFlow(QListWidget::TopToBottom);
    buttonList->setMouseTracking(true);
    //buttonList->setSelectionMode(QAbstractItemView::NoSelection);
    // http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlistview

    for (int i = 0; i != static_cast<int>(Button::Type::last); ++i) {
        auto t = static_cast<Button::Type>(i);
        QListWidgetItem *buttonItem = new QListWidgetItem(buttonList);
        buttonItem->setIcon(Button::getIcon(t));
        buttonItem->setText(Button::typeName[t]);
        buttonItem->setToolTip(Button::typeTooltip[t]);
        buttonItem->setCheckState(Qt::Unchecked);
    }

    QSettings settings;
    // http://www.qtcentre.org/threads/52957-QColor-vector-in-QSettings

    // black white icons
    // color interface

    // buttons available

    // path save
    // color paint tools
    show();
}
