// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    ClickableLabel(QString s, QWidget* parent = nullptr);

signals:
    void clicked();

private:
    void mousePressEvent(QMouseEvent*);
};
