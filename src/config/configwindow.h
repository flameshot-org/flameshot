// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#pragma once

#include <QTabWidget>

class FileNameEditor;
class ShortcutsWidget;
class GeneneralConf;
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
    GeneneralConf* m_generalConfig;
    VisualsEditor* m_visuals;
    QFileSystemWatcher* m_configWatcher;
};
