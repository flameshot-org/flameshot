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
#include "src/utils/confighandler.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QFileSystemWatcher>
#include <QDebug>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget *parent) : QTabWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(400, 490);
    setWindowIcon(QIcon(":img/flameshot.png"));
    setWindowTitle(tr("Configuration"));

    auto changedSlot = [this](QString s){
        Q_UNUSED(s);
        this->m_configWatcher->removePath(s);
        this->m_configWatcher->addPath(s);
        if(!this->hasFocus()) {
            Q_EMIT updateComponents();
        }
    };
    m_configWatcher = new QFileSystemWatcher(this);
    m_configWatcher->addPath(ConfigHandler().getConfigFilePath());
    connect(m_configWatcher, &QFileSystemWatcher::fileChanged,
            this, changedSlot);

    QColor background = this->palette().background().color();
    bool isWhite = CaptureButton::iconIsWhiteByColor(background);
    QString modifier = isWhite ? ":img/configWhite/" : ":img/configBlack/";

    // visuals
    auto visuals = new QWidget();
    QVBoxLayout *layoutUI= new QVBoxLayout();
    visuals->setLayout(layoutUI);
    m_colorEditor = new UIcolorEditor();
    layoutUI->addWidget(m_colorEditor);

    auto boxButtons = new QGroupBox();
    boxButtons->setTitle(tr("Button Selection"));
    auto listLayout = new QVBoxLayout(boxButtons);
    m_buttonList = new ButtonListView();
    layoutUI->addWidget(boxButtons);
    listLayout->addWidget(m_buttonList);

    QPushButton* setAllButtons = new QPushButton(tr("Select All"));
    connect(setAllButtons, &QPushButton::clicked,
            m_buttonList, &ButtonListView::selectAll);
    listLayout->addWidget(setAllButtons);

    addTab(visuals, tr("Interface"));
    setTabIcon(0, QIcon(modifier + "graphics.png"));

    // filename
    m_filenameEditor = new FileNameEditor();
    addTab(m_filenameEditor, tr("Filename Editor"));
    setTabIcon(1, QIcon(modifier + "name_edition.png"));

    // general
    m_generalConfig = new GeneneralConf();
    addTab(m_generalConfig, tr("General"));
    setTabIcon(2, QIcon(modifier + "config.png"));

    // connect update sigslots
    connect(this, &ConfigWindow::updateChildren,
            m_filenameEditor, &FileNameEditor::updateComponents);
    connect(this, &ConfigWindow::updateChildren,
            m_colorEditor, &UIcolorEditor::updateComponents);
    connect(this, &ConfigWindow::updateChildren,
            m_buttonList, &ButtonListView::updateComponents);
    connect(this, &ConfigWindow::updateChildren,
            m_generalConfig, &GeneneralConf::updateComponents);
}

void ConfigWindow::updateComponents() {
    Q_EMIT updateChildren();
}

void ConfigWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
            close();
    }
}
