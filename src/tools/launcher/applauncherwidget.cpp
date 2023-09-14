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
#include <QStandardPaths>
#include <QTabWidget>

namespace {
#if defined(Q_OS_WIN)
QMap<QString, QString> catIconNames({ { "Graphics", "image.svg" },
                                      { "Utility", "apps.svg" } });
}
#else
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
#endif

AppLauncherWidget::AppLauncherWidget(const QPixmap& p, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(p)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Open With"));

    m_keepOpen = ConfigHandler().keepOpenAppLauncher();

#if defined(Q_OS_WIN)
    QDir userAppsFolder(
      QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)
        .at(0));
    m_parser.processDirectory(userAppsFolder);

    QString dir(m_parser.getAllUsersStartMenuPath());
    if (!dir.isEmpty()) {
        QDir allUserAppsFolder(dir);
        m_parser.processDirectory(allUserAppsFolder);
    }
#else
    QString dirLocal = QDir::homePath() + "/.local/share/applications/";
    QDir appsDirLocal(dirLocal);
    m_parser.processDirectory(appsDirLocal);

    QString dir = QStringLiteral("/usr/share/applications/");
    QDir appsDir(dir);
    m_parser.processDirectory(appsDir);
#endif

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
    // Heuristically, if there is a % in the command we assume it is the file
    // name slot
    QString command = index.data(Qt::UserRole).toString();
#if defined(Q_OS_WIN)
    // Do not split on Windows, since file path can contain spaces
    // and % is not used in lnk files
    QStringList prog_args;
    prog_args << command;
#else
    QStringList prog_args = command.split(" ");
#endif
    // no quotes because it is going in an array!
    if (command.contains("%")) {
        // but that means we need to substitute IN the array not the string!
        for (auto& i : prog_args) {
            if (i.contains("%"))
                i.replace(QRegExp("(\\%.)"), m_tempFile);
        }
    } else {
        // we really should append the file name if there
        prog_args.append(m_tempFile); // were no replacements
    }
    QString app_name = prog_args.at(0);
    bool inTerminal =
      index.data(Qt::UserRole + 1).toBool() || m_terminalCheckbox->isChecked();
    if (inTerminal) {
        bool ok = TerminalLauncher::launchDetached(command);
        if (!ok) {
            QMessageBox::about(
              this, tr("Error"), tr("Unable to launch in terminal."));
        }
    } else {
        QFileInfo fi(m_tempFile);
        QString workingDir = fi.absolutePath();
        prog_args.removeAt(0); // strip program name out
        QProcess::startDetached(app_name, prog_args, workingDir);
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

#if defined(Q_OS_WIN)
        QColor background = this->palette().window().color();
        bool isDark = ColorUtils::colorIsDark(background);
        QString modifier =
          isDark ? PathInfo::whiteIconPath() : PathInfo::blackIconPath();
        m_tabWidget->addTab(
          itemsWidget, QIcon(modifier + iconName), QLatin1String(""));
#else
        m_tabWidget->addTab(
          itemsWidget, QIcon::fromTheme(iconName), QLatin1String(""));
#endif
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
    for (const QString& name : qAsConst(multimediaNames)) {
        if (!m_appsMap.contains(name)) {
            continue;
        }
        for (const auto& i : m_appsMap[name]) {
            if (!multimediaList.contains(i)) {
                multimediaList.append(i);
            }
        }
        m_appsMap.remove(name);
    }

    if (!multimediaList.isEmpty()) {
        m_appsMap.insert(QStringLiteral("Multimedia"), multimediaList);
    }
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
    } else if (keyEvent->key() == Qt::Key_Return) {
        auto* widget = (QListWidget*)m_tabWidget->currentWidget();
        if (m_filterList->isVisible())
            widget = m_filterList;
        auto* item = widget->currentItem();
        if (item == nullptr) {
            item = widget->item(0);
            widget->setCurrentItem(item);
        }
        QModelIndex const idx = widget->currentIndex();
        AppLauncherWidget::launch(idx);
    } else {
        QWidget::keyPressEvent(keyEvent);
    }
}
