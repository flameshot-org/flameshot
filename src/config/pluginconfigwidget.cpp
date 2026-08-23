// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "pluginconfigwidget.h"
#include "plugins/pluginmanager.h"

#include <QCheckBox>
#include <QDesktopServices>
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
    mainLayout->addWidget(m_pluginTable);

    // Status label
    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);

    // Bottom action buttons
    auto* btnLayout = new QHBoxLayout();
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
          tr("No plugins discovered. Place plugin (.so / .dll / .dylib) files "
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

void PluginConfigWidget::onPluginToggle(int row)
{
    Q_UNUSED(row);
}
