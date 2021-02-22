// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QPushButton>

class CaptureButton : public QPushButton
{
    Q_OBJECT

public:
    CaptureButton() = delete;
    CaptureButton(QWidget* parent = nullptr);
    CaptureButton(const QString& text, QWidget* parent = nullptr);
    CaptureButton(const QIcon& icon,
                  const QString& text,
                  QWidget* parent = nullptr);

    static QString globalStyleSheet();

    QString styleSheet() const;

    void setColor(const QColor& c);

private:
    static QColor m_mainColor;

    void init();
};
