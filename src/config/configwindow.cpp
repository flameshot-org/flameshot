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

#include "configwindow.h"
#include "src/capture/widgets/capturebutton.h"
#include "src/config/geneneralconf.h"
#include "src/config/filenameeditor.h"
#include "src/config/strftimechooserwidget.h"
#include "src/utils/confighandler.h"
#include "src/config/visualseditor.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QFileSystemWatcher>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget *parent) : QTabWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    const int size = CaptureButton::buttonBaseSize() * 12;
    setMinimumSize(size, size);
    setWindowIcon(QIcon(":img/flameshot.png"));
    setWindowTitle(tr("Configuration"));

    auto changedSlot = [this](QString s){
        QStringList files = m_configWatcher->files();
        if (!files.contains(s)) {
            this->m_configWatcher->addPath(s);
        }
        Q_EMIT updateChildren();
    };
    m_configWatcher = new QFileSystemWatcher(this);
    m_configWatcher->addPath(ConfigHandler().configFilePath());
    connect(m_configWatcher, &QFileSystemWatcher::fileChanged,
            this, changedSlot);

    QColor background = this->palette().background().color();
    bool isWhite = CaptureButton::iconIsWhiteByColor(background);
    QString modifier = isWhite ? ":img/configWhite/" : ":img/configBlack/";

    // visuals
    m_visuals = new VisualsEditor();
    addTab(m_visuals, QIcon(modifier + "graphics.png"),
           tr("Interface"));

    // filename
    m_filenameEditor = new FileNameEditor();
    addTab(m_filenameEditor, QIcon(modifier + "name_edition.png"),
           tr("Filename Editor"));

    // general
    m_generalConfig = new GeneneralConf();
    addTab(m_generalConfig, QIcon(modifier + "config.png"),
           tr("General"));

    // connect update sigslots
    connect(this, &ConfigWindow::updateChildren,
            m_filenameEditor, &FileNameEditor::updateComponents);
    connect(this, &ConfigWindow::updateChildren,
            m_visuals, &VisualsEditor::updateComponents);
    connect(this, &ConfigWindow::updateChildren,
            m_generalConfig, &GeneneralConf::updateComponents);
}

void ConfigWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
            close();
    }
}
