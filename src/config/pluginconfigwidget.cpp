// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "pluginconfigwidget.h"
#include "plugins/pluginmanager.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>

PluginConfigWidget::PluginConfigWidget(QWidget* parent)
  : QWidget(parent)
{
    setupUi();
    populatePlugins();

    connect(PluginManager::instance(),
            &PluginManager::pluginsChanged,
            this,
            &PluginConfigWidget::populatePlugins);
}

void PluginConfigWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Header info description
    auto* infoLabel =
      new QLabel(tr("Flameshot plugins extend screenshot capabilities with "
                    "custom actions, tools, "
                    "and third-party integrations (e.g. OCR, AI analysis, QR "
                    "scanning, custom uploaders)."),
                 this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Plugin table
    m_pluginTable = new QTableWidget(this);
    m_pluginTable->setColumnCount(5);
    m_pluginTable->setHorizontalHeaderLabels({ tr("Enabled"),
                                               tr("Name"),
                                               tr("Version"),
                                               tr("Author"),
                                               tr("Description") });
    m_pluginTable->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents);
    m_pluginTable->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::ResizeToContents);
    m_pluginTable->horizontalHeader()->setSectionResizeMode(
      2, QHeaderView::ResizeToContents);
    m_pluginTable->horizontalHeader()->setSectionResizeMode(
      3, QHeaderView::ResizeToContents);
    m_pluginTable->horizontalHeader()->setSectionResizeMode(
      4, QHeaderView::Stretch);
    m_pluginTable->verticalHeader()->setVisible(false);
    m_pluginTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pluginTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pluginTable->setAlternatingRowColors(true);
    m_pluginTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    m_pluginTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pluginTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pluginTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pluginTable->setMinimumHeight(180);
    mainLayout->addWidget(m_pluginTable, 1);

    // Status label
    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);

    // Bottom action buttons
    auto* btnLayout = new QHBoxLayout();

    m_installBtn = new QPushButton(tr("➕ Install Plugin..."), this);
    connect(m_installBtn,
            &QPushButton::clicked,
            this,
            &PluginConfigWidget::installPlugin);
    btnLayout->addWidget(m_installBtn);

    m_removeBtn = new QPushButton(tr("🚫 Move to Disabled"), this);
    connect(m_removeBtn,
            &QPushButton::clicked,
            this,
            &PluginConfigWidget::removeSelectedPlugin);
    btnLayout->addWidget(m_removeBtn);

    m_openFolderBtn = new QPushButton(tr("Open Plugins Folder"), this);
    connect(m_openFolderBtn,
            &QPushButton::clicked,
            this,
            &PluginConfigWidget::openPluginsFolder);
    btnLayout->addWidget(m_openFolderBtn);

    m_reloadBtn = new QPushButton(tr("Reload Plugins"), this);
    connect(m_reloadBtn,
            &QPushButton::clicked,
            this,
            &PluginConfigWidget::reloadPlugins);
    btnLayout->addWidget(m_reloadBtn);

    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}

void PluginConfigWidget::populatePlugins()
{
    const auto& plugins = PluginManager::instance()->plugins();
    m_pluginTable->setRowCount(plugins.size());

    for (int row = 0; row < plugins.size(); ++row) {
        const auto& meta = plugins.at(row);

        // Checkbox widget for Enabled state
        auto* checkWidget = new QWidget(m_pluginTable);
        auto* checkLayout = new QHBoxLayout(checkWidget);
        checkLayout->setContentsMargins(0, 0, 0, 0);
        checkLayout->setAlignment(Qt::AlignCenter);

        auto* checkBox = new QCheckBox(checkWidget);
        checkBox->setChecked(meta.enabled);
        connect(checkBox,
                &QCheckBox::toggled,
                this,
                [this, id = meta.id](bool checked) {
                    PluginManager::instance()->setPluginEnabled(id, checked);
                });
        checkLayout->addWidget(checkBox);
        m_pluginTable->setCellWidget(row, 0, checkWidget);

        // Name with icon
        auto* nameItem = new QTableWidgetItem(meta.name);
        nameItem->setData(Qt::UserRole, meta.id);
        if (!meta.icon.isNull()) {
            nameItem->setIcon(meta.icon);
        }
        m_pluginTable->setItem(row, 1, nameItem);

        // Version
        m_pluginTable->setItem(row, 2, new QTableWidgetItem(meta.version));

        // Author
        m_pluginTable->setItem(row, 3, new QTableWidgetItem(meta.author));

        // Description
        m_pluginTable->setItem(row, 4, new QTableWidgetItem(meta.description));
    }

    if (plugins.isEmpty()) {
        m_statusLabel->setText(
          tr("No plugins discovered. Use 'Install Plugin...' or place plugin files "
             "into the plugins folder."));
    } else {
        m_statusLabel->setText(
          tr("%1 plugin(s) found in search path.").arg(plugins.size()));
    }
}

void PluginConfigWidget::updateComponents()
{
    populatePlugins();
}

void PluginConfigWidget::openPluginsFolder()
{
    QString path = PluginManager::instance()->userPluginsDirectory();
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void PluginConfigWidget::reloadPlugins()
{
    PluginManager::instance()->reloadPlugins();
}

void PluginConfigWidget::installPlugin()
{
    QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Select Flameshot Plugin Library"),
      QString(),
      tr("Flameshot Plugins (*.so *.dll *.dylib);;All Files (*)"));

    if (filePath.isEmpty()) {
        return;
    }

    QString errorMsg;
    if (PluginManager::instance()->installPlugin(filePath, &errorMsg)) {
        m_statusLabel->setText(
          tr("Successfully installed plugin: %1").arg(QFileInfo(filePath).fileName()));
    } else {
        m_statusLabel->setText(tr("Installation failed: %1").arg(errorMsg));
    }
}

void PluginConfigWidget::removeSelectedPlugin()
{
    int row = m_pluginTable->currentRow();
    if (row < 0) {
        m_statusLabel->setText(tr("Please select a plugin from the table to remove."));
        return;
    }

    auto* item = m_pluginTable->item(row, 1);
    if (!item) {
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    QString errorMsg;
    if (PluginManager::instance()->removePlugin(id, &errorMsg)) {
        m_statusLabel->setText(
          tr("Plugin '%1' moved to disabled directory.").arg(item->text()));
    } else {
        m_statusLabel->setText(tr("Failed to remove plugin: %1").arg(errorMsg));
    }
}

void PluginConfigWidget::onPluginToggle(int row)
{
    Q_UNUSED(row);
}
