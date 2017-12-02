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

#include "applauncherwidget.h"
#include "src/utils/desktopfileparse.h"
#include "src/utils/filenamehandler.h"
#include "src/capture/workers/launcher/launcheritemdelegate.h"
#include "src/utils/confighandler.h"
#include <QDir>
#include <QVector>
#include <QProcess>
#include <QPixmap>
#include <QListView>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QCheckBox>

AppLauncherWidget::AppLauncherWidget(const QPixmap &p, QWidget *parent):
    QWidget(parent), m_pixmap(p)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_keepOpen = ConfigHandler().keepOpenAppLauncherValue();

    QString dir = "/usr/share/applications/";
    QString dirLocal = "~/.local/share/applications/";
    QDir appsDirLocal(dirLocal);
    QDir appsDir(dir);

    QStringList entries = appsDir.entryList(QDir::NoDotAndDotDot | QDir::Files);
    QStringList entriesLocal = appsDirLocal.entryList(QDir::NoDotAndDotDot | QDir::Files);

    DesktopFileParse parser;
    bool ok;
    QList<DesktopAppData> appList;
    for (QString file: entries){
        DesktopAppData app = parser.parseDesktopFile(dir + file, ok);
        if (ok) {
            appList.append(app);
        }
    }
    for (QString file: entriesLocal){
        DesktopAppData app = parser.parseDesktopFile(dirLocal + file, ok);
        if (ok) {
            appList.append(app);
        }
    }
    auto layout = new QVBoxLayout(this);
    auto *listView = new QListWidget(this);
    listView->setItemDelegate(new launcherItemDelegate());
    listView->setViewMode(QListWidget::IconMode);
    listView->setResizeMode(QListView::Adjust);
    listView->setSpacing(4);
    listView->setFlow(QListView::LeftToRight);
    listView->setDragEnabled(false);

    for (auto app: appList) {
        QListWidgetItem *buttonItem = new QListWidgetItem(listView);
        buttonItem->setData(Qt::DecorationRole, app.icon);
        buttonItem->setData(Qt::DisplayRole, app.name);
        buttonItem->setData(Qt::UserRole, app.exec);
        QColor foregroundColor = this->palette().color(QWidget::foregroundRole());
        buttonItem->setForeground(foregroundColor);

        buttonItem->setIcon(app.icon);
        buttonItem->setText(app.name);
        buttonItem->setToolTip(app.description);
    }
    connect(listView, &QListWidget::clicked, this, &AppLauncherWidget::launch);

    m_checkbox = new QCheckBox("Keep open after selection", this);
    connect(m_checkbox, &QCheckBox::clicked, this, &AppLauncherWidget::checkboxClicked);

    layout->addWidget(listView);
    layout->addWidget(m_checkbox);
}

void AppLauncherWidget::launch(const QModelIndex &index) {
    m_tempFile = FileNameHandler().generateAbsolutePath("/tmp") + ".png";
    bool ok = m_pixmap.save(m_tempFile);
    if (!ok) {
        // TO DO
        return;
    }
    QString command = index.data(Qt::UserRole).toString().replace(
                QRegExp("(\%.)"), m_tempFile);
    QProcess::startDetached(command);
    if (!m_keepOpen) {
        close();
    }
}

void AppLauncherWidget::checkboxClicked(const bool enabled) {
    m_keepOpen = enabled;
    ConfigHandler().setKeepOpenAppLauncher(enabled);
    m_checkbox->setChecked(enabled);
}
