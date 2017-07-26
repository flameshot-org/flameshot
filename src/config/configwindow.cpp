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
#include "src/capture/capturebutton.h"
#include "src/config/buttonlistview.h"
#include "src/config/uicoloreditor.h"
#include "src/config/geneneralconf.h"
#include "src/config/filenameeditor.h"
#include "src/config/strftimechooserwidget.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget *parent) : QTabWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(395, 490);
    setWindowIcon(QIcon(":img/flameshot.png"));
    setWindowTitle(tr("Configuration"));

    QColor background = this->palette().background().color();
    bool isWhite = CaptureButton::iconIsWhiteByColor(background);
    QString modifier = isWhite ? ":img/configWhite/" : ":img/configBlack/";

    // visuals
    auto visuals = new QWidget();
    QVBoxLayout *layoutUI= new QVBoxLayout();
    visuals->setLayout(layoutUI);
    layoutUI->addWidget(new UIcolorEditor());

    auto boxButtons = new QGroupBox();
    boxButtons->setTitle(tr("Button Selection"));
    auto listLayout = new QVBoxLayout(boxButtons);
    auto buttonList = new ButtonListView();
    layoutUI->addWidget(boxButtons);
    listLayout->addWidget(buttonList);

    QPushButton* setAllButtons = new QPushButton(tr("Select All"));
    connect(setAllButtons, &QPushButton::clicked,
            buttonList, &ButtonListView::selectAll);
    listLayout->addWidget(setAllButtons);

    addTab(visuals, tr("Interface"));
    setTabIcon(0, QIcon(modifier + "graphics.png"));

    // filename
    addTab(new FileNameEditor(), tr("Filename Editor"));
    setTabIcon(1, QIcon(modifier + "name_edition.png"));

    // general
    addTab(new GeneneralConf(), tr("General"));
    setTabIcon(2, QIcon(modifier + "config.png"));
}

void ConfigWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
            close();
    }
}
