// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "widgets/capture/capturetoolbutton.h"
#include <QListWidget>

class ButtonListView : public QListWidget
{
public:
    explicit ButtonListView(QWidget* parent = nullptr);

public slots:
    void reload();
    void selectAll();
    void updateComponents();

public:
    QString selectedExternalPluginId() const;

private slots:
    void reverseItemCheck(QListWidgetItem*);

protected:
    void initButtonList();

private:
    QList<CaptureTool::Type> m_listButtons;

    void updateActiveButtons(QListWidgetItem*);
};
