// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QTimer;

class NotifierBox : public QWidget
{
    Q_OBJECT
public:
    explicit NotifierBox(QWidget* parent = nullptr);

protected:
    virtual void enterEvent(QEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

signals:
    void hidden();

public slots:
    void showMessage(const QString& msg);
    void showColor(const QColor& color);

private:
    QTimer* m_timer;
    QString m_message;
    QColor m_bgColor;
    QColor m_foregroundColor;

    void hideEvent(QHideEvent* event) override;
};
