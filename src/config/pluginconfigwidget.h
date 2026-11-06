// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QWidget>

class QTableWidget;
class QLabel;
class QPushButton;

class PluginConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginConfigWidget(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void openPluginsFolder();
    void reloadPlugins();
    void installPlugin();
    void removeSelectedPlugin();
    void onPluginToggle(int row);

private:
    void setupUi();
    void populatePlugins();

    QTableWidget* m_pluginTable{ nullptr };
    QLabel* m_statusLabel{ nullptr };
    QPushButton* m_installBtn{ nullptr };
    QPushButton* m_removeBtn{ nullptr };
    QPushButton* m_openFolderBtn{ nullptr };
    QPushButton* m_reloadBtn{ nullptr };
};
