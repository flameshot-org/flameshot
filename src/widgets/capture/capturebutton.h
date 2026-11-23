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
    static QString styleSheetForColor(const QColor& c);

    QString styleSheet() const;

    void setColor(const QColor& c);
    void setInheritedColor(const QColor& c);
    const QColor& color() const { return m_color; }

protected:
    QColor m_color;

private:
    void init();
};
