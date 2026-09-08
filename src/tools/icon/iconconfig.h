// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QVBoxLayout;

class IconConfig : public QWidget
{
    Q_OBJECT
public:
    explicit IconConfig(QWidget* parent = nullptr);

    void setSymbol(const QString& symbol);

signals:
    void symbolChanged(const QString& symbol);

private:
    QVBoxLayout* m_layout;
    QComboBox* m_symbolCB;
};
