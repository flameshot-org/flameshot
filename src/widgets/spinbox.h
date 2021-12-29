// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QColor>
#include <QSpinBox>

class SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget* parent = nullptr);
    void updateWidget();

protected:
    int valueFromText(const QString& text) const override;
    QString textFromValue(int value) const override;

private:
    void initSpinbox();

    QVector<QColor> m_colorList;
};