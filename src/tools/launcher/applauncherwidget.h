// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/utils/desktopfileparse.h"
#include <QMap>
#include <QWidget>

class QTabWidget;
class QCheckBox;
class QVBoxLayout;
class QLineEdit;
class QListWidget;

class AppLauncherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppLauncherWidget(const QPixmap& p, QWidget* parent = nullptr);

private slots:
    void launch(const QModelIndex& index);
    void checkboxClicked(const bool enabled);
    void searchChanged(const QString& text);

private:
    void initListWidget();
    void initAppMap();
    void configureListView(QListWidget* widget);
    void addAppsToListWidget(QListWidget* widget,
                             const QVector<DesktopAppData>& appList);
    void keyPressEvent(QKeyEvent* keyEvent) override;

    DesktopFileParser m_parser;
    QPixmap m_pixmap;
    QString m_tempFile;
    bool m_keepOpen;
    QMap<QString, QVector<DesktopAppData>> m_appsMap;
    QCheckBox* m_keepOpenCheckbox;
    QCheckBox* m_terminalCheckbox;
    QVBoxLayout* m_layout;
    QLineEdit* m_lineEdit;
    QListWidget* m_filterList;
    QTabWidget* m_tabWidget;
};
