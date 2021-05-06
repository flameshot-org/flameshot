// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/widgets/capture/capturetoolbutton.h"
#include <QListWidget>

class ButtonListView : public QListWidget
{
public:
    explicit ButtonListView(QWidget* parent = nullptr);

public slots:
    void selectAll();
    void updateComponents();

private slots:
    void reverseItemCheck(QListWidgetItem*);

protected:
    void initButtonList();

private:
    QVector<CaptureToolButton::ButtonType> m_listButtons;
    QMap<QString, CaptureToolButton::ButtonType> m_buttonTypeByName;

    void updateActiveButtons(QListWidgetItem*);
};
