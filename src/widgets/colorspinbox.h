// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#pragma once

#include <QColor>
#include <QSpinBox>

class ColorSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit ColorSpinBox(QWidget* parent = nullptr);
    void updateWidget();

protected:
    int valueFromText(const QString& text) const override;
    QString textFromValue(int value) const override;

private:
    void initColorSpinbox();

    QVector<QColor> m_colorList;
};