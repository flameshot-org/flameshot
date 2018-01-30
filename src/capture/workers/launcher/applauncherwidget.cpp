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

#include "applauncherwidget.h"
#include "src/utils/filenamehandler.h"
#include "src/capture/workers/launcher/launcheritemdelegate.h"
#include "src/capture/widgets/capturebutton.h"
#include "src/utils/confighandler.h"
#include "terminallauncher.h"
#include <QDir>
#include <QList>
#include <QProcess>
#include <QPixmap>
#include <QListView>
#include <QTabWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>

namespace {

QMap<QString, QString> catIconNames({
            { "Multimedia", "applications-multimedia" },
            { "Development","applications-development" },
            { "Graphics",    "applications-graphics" },
            { "Network",    "preferences-system-network" },
            { "Office",        "applications-office" },
            { "Science",    "applications-science" },
            { "Settings",    "preferences-desktop" },
            { "System",        "preferences-system" },
            { "Utility",    "applications-utilities" }
                                    });
}

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

    initAppMap();
    initListWidget();

    m_terminalCheckbox = new QCheckBox(tr("Launch in terminal"), this);
    m_keepOpenCheckbox = new QCheckBox(tr("Keep open after selection"), this);
    m_keepOpenCheckbox->setChecked(ConfigHandler().keepOpenAppLauncherValue());
    connect(m_keepOpenCheckbox, &QCheckBox::clicked, this, &AppLauncherWidget::checkboxClicked);

    // search items
    m_lineEdit = new QLineEdit;
    connect(m_lineEdit, &QLineEdit::textChanged,
            this, &AppLauncherWidget::searchChanged);
    m_filterList = new QListWidget;
    m_filterList->hide();
    configureListView(m_filterList);
    connect(m_filterList, &QListWidget::clicked, this, &AppLauncherWidget::launch);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_filterList);
    m_layout->addWidget(m_tabWidget);
    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(m_keepOpenCheckbox);
    m_layout->addWidget(m_terminalCheckbox);
    m_lineEdit->setFocus();
}

void AppLauncherWidget::launch(const QModelIndex &index) {
    if (!QFileInfo(m_tempFile).isReadable()) {
        m_tempFile = FileNameHandler().generateAbsolutePath(QDir::tempPath()) + ".png";
        bool ok = m_pixmap.save(m_tempFile);
        if (!ok) {
            QMessageBox::about(this, tr("Error"), tr("Unable to write in")
                               + QDir::tempPath());
            return;
        }
    }
    QString command = index.data(Qt::UserRole).toString().replace(
                QRegExp("(\\%.)"), '"' + m_tempFile + '"');
    bool inTerminal = index.data(Qt::UserRole+1).toBool() ||
            m_terminalCheckbox->isChecked();
    if (inTerminal) {
        bool ok = TerminalLauncher::launchDetached(command);
        if (!ok) {
            QMessageBox::about(this, tr("Error"),
                               tr("Unable to launch in terminal."));
        }
    } else {
        QProcess::startDetached(command);
    }
    if (!m_keepOpen) {
        close();
    }
}

void AppLauncherWidget::checkboxClicked(const bool enabled) {
    m_keepOpen = enabled;
    ConfigHandler().setKeepOpenAppLauncher(enabled);
    m_keepOpenCheckbox->setChecked(enabled);
}

void AppLauncherWidget::searchChanged(const QString &text) {
    if (text.isEmpty()) {
        m_filterList->hide();
        m_tabWidget->show();
    } else {
        m_tabWidget->hide();
        m_filterList->show();
        m_filterList->clear();
        QRegExp regexp(text, Qt::CaseInsensitive, QRegExp::Wildcard);
        QVector<DesktopAppData> apps;

        for (auto const& i : catIconNames.toStdMap()) {
            const QString &cat = i.first;
            if (!m_appsMap.contains(cat)) {
                continue;
            }
            const QVector<DesktopAppData> &appList = m_appsMap[cat];
            for (const DesktopAppData &app: appList) {
                if (!apps.contains(app) && (app.name.contains(regexp) ||
                        app.description.contains(regexp) ))
                {
                    apps.append(app);
                }
            }
        }
        addAppsToListWidget(m_filterList, apps);
    }
}

void AppLauncherWidget::initListWidget() {
    m_tabWidget = new QTabWidget;
    const int size = CaptureButton::buttonBaseSize();
    m_tabWidget->setIconSize(QSize(size, size));

    for (auto const& i : catIconNames.toStdMap()) {
        const QString &cat = i.first;
        const QString &iconName = i.second;

        if (!m_appsMap.contains(cat)) {
            continue;
        }

        QListWidget *itemsWidget = new QListWidget();
        configureListView(itemsWidget);

        const QVector<DesktopAppData> &appList = m_appsMap[cat];
        addAppsToListWidget(itemsWidget, appList);

        m_tabWidget->addTab(itemsWidget, QIcon::fromTheme(iconName), "");
        m_tabWidget->setTabToolTip(m_tabWidget->count(), cat);
        if (cat == "Graphics") {
            m_tabWidget->setCurrentIndex(m_tabWidget->count() -1);
        }
    }
}

void AppLauncherWidget::initAppMap() {
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

    m_appsMap = m_parser.getAppsByCategory(categories);

    // Unify multimedia.
    QVector<DesktopAppData> multimediaList;
    QStringList multimediaNames;
    multimediaNames << "AudioVideo" << "Audio" << "Video";
    for (const QString &name : multimediaNames) {
        if(!m_appsMap.contains(name)) {
            continue;
        }
        for (auto i : m_appsMap[name]) {
            if (!multimediaList.contains(i)) {
                multimediaList.append(i);
            }
        }
        m_appsMap.remove(name);
    }
    m_appsMap.insert("Multimedia", multimediaList);
}

void AppLauncherWidget::configureListView(QListWidget *widget) {
    widget->setItemDelegate(new LauncherItemDelegate());
    widget->setViewMode(QListWidget::IconMode);
    widget->setResizeMode(QListView::Adjust);
    widget->setSpacing(4);
    widget->setFlow(QListView::LeftToRight);
    widget->setDragEnabled(false);
    widget->setMinimumWidth(CaptureButton::buttonBaseSize() * 11);
    connect(widget, &QListWidget::clicked,
            this, &AppLauncherWidget::launch);
}

void AppLauncherWidget::addAppsToListWidget(
        QListWidget *widget, const QVector<DesktopAppData> &appList)
{
    for (const DesktopAppData &app: appList) {
        QListWidgetItem *buttonItem = new QListWidgetItem(widget);
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
}
