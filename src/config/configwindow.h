// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QTabWidget>

class FileNameEditor;
class ShortcutsWidget;
class GeneralConf;
class QFileSystemWatcher;
class VisualsEditor;
class PluginConfigWidget;
class QWidget;

class ConfigWindow : public QWidget
{
    Q_OBJECT
public:
    enum TabIndex
    {
        GeneralTab = 0,
        VisualsTab = 1,
        FilenameTab = 2,
        ShortcutsTab = 3,
        PluginsTab = 4
    };

    explicit ConfigWindow(QWidget* parent = nullptr,
                          TabIndex initialTab = GeneralTab);
    void setCurrentTab(TabIndex tab);

signals:
    void updateChildren();

protected:
    void keyPressEvent(QKeyEvent*);

private:
    QTabWidget* m_tabWidget;

    FileNameEditor* m_filenameEditor;
    QWidget* m_filenameEditorTab;

    ShortcutsWidget* m_shortcuts;
    QWidget* m_shortcutsTab;

    GeneralConf* m_generalConfig;
    QWidget* m_generalConfigTab;

    VisualsEditor* m_visuals;
    QWidget* m_visualsTab;

    PluginConfigWidget* m_pluginsWidget;
    QWidget* m_pluginsTab;

    void initErrorIndicator(QWidget* tab, QWidget* widget);
};
