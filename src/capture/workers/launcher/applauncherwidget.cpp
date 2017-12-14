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
#include "src/utils/filenamehandler.h"
#include "src/capture/workers/launcher/launcheritemdelegate.h"
#include "src/utils/confighandler.h"
#include "terminallauncher.h"
#include "moreappswidget.h"
#include <QDir>
#include <QList>
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
	setWindowIcon(QIcon(":img/flameshot.png"));
	setWindowTitle(tr("Open With"));

    m_keepOpen = ConfigHandler().keepOpenAppLauncherValue();

	QString dirLocal = QDir::homePath() + "/.local/share/applications/";
	QDir appsDirLocal(dirLocal);
	m_parser.processDirectory(appsDirLocal);

	QString dir = "/usr/share/applications/";
	QDir appsDir(dir);
	m_parser.processDirectory(appsDir);

	m_layout = new QVBoxLayout(this);
	QListWidget *listView = new QListWidget(this);
	listView->setItemDelegate(new LauncherItemDelegate());
    listView->setViewMode(QListWidget::IconMode);
    listView->setResizeMode(QListView::Adjust);
    listView->setSpacing(4);
    listView->setFlow(QListView::LeftToRight);
    listView->setDragEnabled(false);
    listView->setMinimumSize(375, 210);

	QList<DesktopAppData> appList = m_parser.getAppsByCategory("Graphics");
	appList.append(DesktopAppData(QObject::tr("Other Application"), "", ".",
								  QIcon::fromTheme("applications-other")
					   ));

    for (auto app: appList) {
        QListWidgetItem *buttonItem = new QListWidgetItem(listView);
        buttonItem->setData(Qt::DecorationRole, app.icon);
        buttonItem->setData(Qt::DisplayRole, app.name);
        buttonItem->setData(Qt::UserRole, app.exec);
		buttonItem->setData(Qt::UserRole+1, app.showInTerminal);
        QColor foregroundColor = this->palette().color(QWidget::foregroundRole());
        buttonItem->setForeground(foregroundColor);

        buttonItem->setIcon(app.icon);
        buttonItem->setText(app.name);
        buttonItem->setToolTip(app.description);
    }
    connect(listView, &QListWidget::clicked, this, &AppLauncherWidget::launch);

    m_checkbox = new QCheckBox("Keep open after selection", this);
    connect(m_checkbox, &QCheckBox::clicked, this, &AppLauncherWidget::checkboxClicked);

	m_layout->addWidget(listView);
	m_layout->addWidget(m_checkbox);
	m_checkbox->setChecked(ConfigHandler().keepOpenAppLauncherValue());
}

void AppLauncherWidget::launch(const QModelIndex &index) {
	QString command = index.data(Qt::UserRole).toString().replace(
				QRegExp("(\%.)"), m_tempFile);
	if (command == ".") {
		MoreAppsWidget *widget = new MoreAppsWidget(m_pixmap, m_parser);
		connect(widget, &MoreAppsWidget::appClicked,
				this, &AppLauncherWidget::launch);
		m_layout->takeAt(0)->widget()->deleteLater();
		m_layout->insertWidget(0, widget);
	} else {
		m_tempFile = FileNameHandler().generateAbsolutePath("/tmp") + ".png";
		bool ok = m_pixmap.save(m_tempFile);
		if (!ok) {
			// TO DO
			return;
		}
		bool inTerminal = index.data(Qt::UserRole+1).toBool();
		if (inTerminal) {
			ok = TerminalLauncher::launchDetached(command);
			if (!ok) {
				// TO DO
			}
		} else {
			QProcess::startDetached(command);
		}
	}
	if (!m_keepOpen) {
		close();
	}
}

void AppLauncherWidget::checkboxClicked(const bool enabled) {
    m_keepOpen = enabled;
    ConfigHandler().setKeepOpenAppLauncher(enabled);
    m_checkbox->setChecked(enabled);
}
