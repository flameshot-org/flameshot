// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class StrftimeChooserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StrftimeChooserWidget(QWidget* parent = nullptr);

signals:
    void variableEmitted(const QString&);

private:
    static QMap<QString, QString> m_buttonData;
};
