// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "applauncherwidget.h"
#include "src/tools/launcher/launcheritemdelegate.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/globalvalues.h"
#include "terminallauncher.h"
#include <QCheckBox>
#include <QDir>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QListView>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QTabWidget>

namespace {

QMap<QString, QString> catIconNames(
  { { "Multimedia", "applications-multimedia" },
    { "Development", "applications-development" },
    { "Graphics", "applications-graphics" },
    { "Network", "preferences-system-network" },
    { "Office", "applications-office" },
    { "Science", "applications-science" },
    { "Settings", "preferences-desktop" },
    { "System", "preferences-system" },
    { "Utility", "applications-utilities" } });
}

AppLauncherWidget::AppLauncherWidget(const QPixmap& p, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(p)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Open With"));

    m_keepOpen = ConfigHandler().keepOpenAppLauncher();

    QString dirLocal = QDir::homePath() + "/.local/share/applications/";
    QDir appsDirLocal(dirLocal);
    m_parser.processDirectory(appsDirLocal);

    QString dir = QStringLiteral("/usr/share/applications/");
    QDir appsDir(dir);
    m_parser.processDirectory(appsDir);

    initAppMap();
    initListWidget();

    m_terminalCheckbox = new QCheckBox(tr("Launch in terminal"), this);
    m_keepOpenCheckbox = new QCheckBox(tr("Keep open after selection"), this);
    m_keepOpenCheckbox->setChecked(ConfigHandler().keepOpenAppLauncher());
    connect(m_keepOpenCheckbox,
            &QCheckBox::clicked,
            this,
            &AppLauncherWidget::checkboxClicked);

    // search items
    m_lineEdit = new QLineEdit;
    connect(m_lineEdit,
            &QLineEdit::textChanged,
            this,
            &AppLauncherWidget::searchChanged);
    m_filterList = new QListWidget;
    m_filterList->hide();
    configureListView(m_filterList);
    connect(
      m_filterList, &QListWidget::clicked, this, &AppLauncherWidget::launch);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_filterList);
    m_layout->addWidget(m_tabWidget);
    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(m_keepOpenCheckbox);
    m_layout->addWidget(m_terminalCheckbox);
    m_lineEdit->setFocus();
}

void AppLauncherWidget::launch(const QModelIndex& index)
{
    if (!QFileInfo(m_tempFile).isReadable()) {
        m_tempFile =
          FileNameHandler().properScreenshotPath(QDir::tempPath(), "png");
        bool ok = m_pixmap.save(m_tempFile);
        if (!ok) {
            QMessageBox::about(
              this, tr("Error"), tr("Unable to write in") + QDir::tempPath());
            return;
        }
    }
    QString command = index.data(Qt::UserRole)
                        .toString()
                        .replace(QRegExp("(\\%.)"), '"' + m_tempFile + '"');

    QString app_name = index.data(Qt::UserRole).toString().split(" ").at(0);
    bool inTerminal =
      index.data(Qt::UserRole + 1).toBool() || m_terminalCheckbox->isChecked();
    if (inTerminal) {
        bool ok = TerminalLauncher::launchDetached(command);
        if (!ok) {
            QMessageBox::about(
              this, tr("Error"), tr("Unable to launch in terminal."));
        }
    } else {
        QProcess::startDetached(app_name, { m_tempFile });
    }
    if (!m_keepOpen) {
        close();
    }
}

void AppLauncherWidget::checkboxClicked(const bool enabled)
{
    m_keepOpen = enabled;
    ConfigHandler().setKeepOpenAppLauncher(enabled);
    m_keepOpenCheckbox->setChecked(enabled);
}

void AppLauncherWidget::searchChanged(const QString& text)
{
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
            const QString& cat = i.first;
            if (!m_appsMap.contains(cat)) {
                continue;
            }
            const QVector<DesktopAppData>& appList = m_appsMap[cat];
            for (const DesktopAppData& app : appList) {
                if (!apps.contains(app) && (app.name.contains(regexp) ||
                                            app.description.contains(regexp))) {
                    apps.append(app);
                }
            }
        }
        addAppsToListWidget(m_filterList, apps);
    }
}

void AppLauncherWidget::initListWidget()
{
    m_tabWidget = new QTabWidget;
    const int size = GlobalValues::buttonBaseSize();
    m_tabWidget->setIconSize(QSize(size, size));

    for (auto const& i : catIconNames.toStdMap()) {
        const QString& cat = i.first;
        const QString& iconName = i.second;

        if (!m_appsMap.contains(cat)) {
            continue;
        }

        auto* itemsWidget = new QListWidget();
        configureListView(itemsWidget);

        const QVector<DesktopAppData>& appList = m_appsMap[cat];
        addAppsToListWidget(itemsWidget, appList);

        m_tabWidget->addTab(
          itemsWidget, QIcon::fromTheme(iconName), QLatin1String(""));
        m_tabWidget->setTabToolTip(m_tabWidget->count(), cat);
        if (cat == QLatin1String("Graphics")) {
            m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
        }
    }
}

void AppLauncherWidget::initAppMap()
{
    QStringList categories({ "AudioVideo",
                             "Audio",
                             "Video",
                             "Development",
                             "Graphics",
                             "Network",
                             "Office",
                             "Science",
                             "Settings",
                             "System",
                             "Utility" });

    m_appsMap = m_parser.getAppsByCategory(categories);

    // Unify multimedia.
    QVector<DesktopAppData> multimediaList;
    QStringList multimediaNames;
    multimediaNames << QStringLiteral("AudioVideo") << QStringLiteral("Audio")
                    << QStringLiteral("Video");
    for (const QString& name : multimediaNames) {
        if (!m_appsMap.contains(name)) {
            continue;
        }
        for (auto i : m_appsMap[name]) {
            if (!multimediaList.contains(i)) {
                multimediaList.append(i);
            }
        }
        m_appsMap.remove(name);
    }
    m_appsMap.insert(QStringLiteral("Multimedia"), multimediaList);
}

void AppLauncherWidget::configureListView(QListWidget* widget)
{
    widget->setItemDelegate(new LauncherItemDelegate());
    widget->setViewMode(QListWidget::IconMode);
    widget->setResizeMode(QListView::Adjust);
    widget->setSpacing(4);
    widget->setFlow(QListView::LeftToRight);
    widget->setDragEnabled(false);
    widget->setMinimumWidth(GlobalValues::buttonBaseSize() * 11);
    connect(widget, &QListWidget::clicked, this, &AppLauncherWidget::launch);
}

void AppLauncherWidget::addAppsToListWidget(
  QListWidget* widget,
  const QVector<DesktopAppData>& appList)
{
    for (const DesktopAppData& app : appList) {
        auto* buttonItem = new QListWidgetItem(widget);
        buttonItem->setData(Qt::DecorationRole, app.icon);
        buttonItem->setData(Qt::DisplayRole, app.name);
        buttonItem->setData(Qt::UserRole, app.exec);
        buttonItem->setData(Qt::UserRole + 1, app.showInTerminal);
        QColor foregroundColor =
          this->palette().color(QWidget::foregroundRole());
        buttonItem->setForeground(foregroundColor);

        buttonItem->setIcon(app.icon);
        buttonItem->setText(app.name);
        buttonItem->setToolTip(app.description);
    }
}

void AppLauncherWidget::keyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(keyEvent);
    }
}
