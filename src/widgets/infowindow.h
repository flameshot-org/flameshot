// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QVBoxLayout;

class InfoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit InfoWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent*);

private:
    void initLabels();
    QVBoxLayout* m_layout;
};
