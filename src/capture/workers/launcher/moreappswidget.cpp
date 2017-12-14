#include "moreappswidget.h"
#include "src/capture/workers/launcher/launcheritemdelegate.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QListView>
#include <QListWidgetItem>
#include <QMap>

// implement search, open custom and choose terminal

MoreAppsWidget::MoreAppsWidget(
		const QPixmap &pixmap,
		const DesktopFileParser &parser,
		QWidget *parent) :
	QWidget(parent), m_pixmap(pixmap)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowIcon(QIcon(":img/flameshot.png"));
	setWindowTitle(tr("Open With"));

	DesktopFileParser ctor_parser = std::move(parser);
	m_layout = new QVBoxLayout(this);
	m_tabs = new QTabWidget;
	m_tabs->setIconSize(QSize(30, 30));
	m_layout->addWidget(m_tabs);

	QStringList categories({"AudioVideo",
							"Audio",
							"Video",
							"Development",
							"Graphics",
							"Network",
							"Office",
							"Science",
							"Settings",
							"System",
							"Utility"});

	QMap<QString, QList<DesktopAppData>> appsMap =
			ctor_parser.getAppsByCategory(categories);

	// Unify multimedia.
	QList<DesktopAppData> multimediaList;
	QStringList multimediaNames;
	multimediaNames << "AudioVideo" << "Audio" << "Video";
	for (const QString &name : multimediaNames) {
		if(!appsMap.contains(name)) {
			continue;
		}
		for (auto i : appsMap[name]) {
			if (!multimediaList.contains(i)) {
				multimediaList.append(i);
			}
		}
		appsMap.remove(name);
	}
	appsMap.insert("Multimedia", multimediaList);

	QMap<QString, QString> catIconNames({
				{ "Multimedia", "applications-multimedia" },
				{ "Development","applications-development" },
				{ "Graphics",	"applications-graphics" },
				{ "Network",	"preferences-system-network" },
				{ "Office",		"applications-office" },
				{ "Science",	"applications-science" },
				{ "Settings",	"preferences-desktop" },
				{ "System",		"preferences-system" },
				{ "Utility",	"applications-utilities" }
										});

	for (auto const& i : catIconNames.toStdMap()) {
		const QString &cat = i.first;
		const QString &iconName = i.second;

		if (!appsMap.contains(cat)) {
			continue;
		}

		QListWidget *listView = new QListWidget();
		listView->setItemDelegate(new LauncherItemDelegate());
		listView->setViewMode(QListWidget::IconMode);
		listView->setResizeMode(QListView::Adjust);
		listView->setSpacing(4);
		listView->setFlow(QListView::LeftToRight);
		listView->setDragEnabled(false);
		listView->setMinimumSize(375, 210);
		connect(listView, &QListWidget::clicked,
				this, &MoreAppsWidget::appClicked);

		QList<DesktopAppData> appList = appsMap[cat];
		for (auto app: appList) {
			QListWidgetItem *buttonItem = new QListWidgetItem(listView);
			buttonItem->setData(Qt::DecorationRole, app.icon);
			buttonItem->setData(Qt::DisplayRole, app.name);
			buttonItem->setData(Qt::UserRole, app.exec);
			buttonItem->setData(Qt::UserRole+1, app.showInTerminal);
			QColor foregroundColor =
					this->palette().color(QWidget::foregroundRole());
			buttonItem->setForeground(foregroundColor);

			buttonItem->setIcon(app.icon);
			buttonItem->setText(app.name);
			buttonItem->setToolTip(app.description);
		}
		m_tabs->addTab(listView, QIcon::fromTheme(iconName), "");
		m_tabs->setTabToolTip(m_tabs->count(), cat);
		if (cat == "Graphics") {
			m_tabs->setCurrentIndex(m_tabs->count() -1);
		}
	}
}
