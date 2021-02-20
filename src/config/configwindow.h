// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QTabWidget>

class FileNameEditor;
class ShortcutsWidget;
class GeneralConf;
class QFileSystemWatcher;
class VisualsEditor;

class ConfigWindow : public QTabWidget
{
    Q_OBJECT
public:
    explicit ConfigWindow(QWidget* parent = nullptr);

signals:
    void updateChildren();

protected:
    void keyPressEvent(QKeyEvent*);

private:
    FileNameEditor* m_filenameEditor;
    ShortcutsWidget* m_shortcuts;
    GeneralConf* m_generalConfig;
    VisualsEditor* m_visuals;
    QFileSystemWatcher* m_configWatcher;
};
