// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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
#include "src/config/filenameeditor.h"
#include "src/config/geneneralconf.h"
#include "src/config/strftimechooserwidget.h"
#include "src/config/visualseditor.h"
#include "src/utils/colorutils.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/utils/pathinfo.h"
#include "src/widgets/capture/capturetoolbutton.h"
#include <QFileSystemWatcher>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget* parent)
  : QTabWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    const int size = GlobalValues::buttonBaseSize() * 12;
    setMinimumSize(size, size);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowTitle(tr("Configuration"));

    auto changedSlot = [this](QString s) {
        QStringList files = m_configWatcher->files();
        if (!files.contains(s)) {
            this->m_configWatcher->addPath(s);
        }
        emit updateChildren();
    };
    m_configWatcher = new QFileSystemWatcher(this);
    m_configWatcher->addPath(ConfigHandler().configFilePath());
    connect(
      m_configWatcher, &QFileSystemWatcher::fileChanged, this, changedSlot);

    QColor background = this->palette().window().color();
    bool isDark = ColorUtils::colorIsDark(background);
    QString modifier =
      isDark ? PathInfo::whiteIconPath() : PathInfo::blackIconPath();

    // visuals
    m_visuals = new VisualsEditor();
    addTab(m_visuals, QIcon(modifier + "graphics.svg"), tr("Interface"));

    // filename
    m_filenameEditor = new FileNameEditor();
    addTab(m_filenameEditor,
           QIcon(modifier + "name_edition.svg"),
           tr("Filename Editor"));

    // general
    m_generalConfig = new GeneneralConf();
    addTab(m_generalConfig, QIcon(modifier + "config.svg"), tr("General"));

    // connect update sigslots
    connect(this,
            &ConfigWindow::updateChildren,
            m_filenameEditor,
            &FileNameEditor::updateComponents);
    connect(this,
            &ConfigWindow::updateChildren,
            m_visuals,
            &VisualsEditor::updateComponents);
    connect(this,
            &ConfigWindow::updateChildren,
            m_generalConfig,
            &GeneneralConf::updateComponents);
}

void ConfigWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }
}
