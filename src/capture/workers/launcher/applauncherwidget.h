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

#pragma once

#include "src/utils/desktopfileparse.h"
#include <QWidget>
#include <QMap>

class QTabWidget;
class QCheckBox;
class QVBoxLayout;
class QLineEdit;
class QListWidget;

class AppLauncherWidget: public QWidget
{
    Q_OBJECT
public:
    explicit AppLauncherWidget(const QPixmap &p, QWidget *parent = nullptr);

private slots:
    void launch(const QModelIndex &index);
    void checkboxClicked(const bool enabled);
    void searchChanged(const QString &text);

private:
    void initListWidget();
    void initAppMap();
    void configureListView(QListWidget *widget);
    void addAppsToListWidget(QListWidget *widget,
                             const QVector<DesktopAppData> &appList);

    DesktopFileParser m_parser;
    QPixmap m_pixmap;
    QString m_tempFile;
    bool m_keepOpen;
    QMap<QString, QVector<DesktopAppData>> m_appsMap;
    QCheckBox *m_keepOpenCheckbox;
    QCheckBox *m_terminalCheckbox;
    QVBoxLayout *m_layout;
    QLineEdit *m_lineEdit;
    QListWidget *m_filterList;
    QTabWidget *m_tabWidget;
};
