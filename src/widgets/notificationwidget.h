// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QLabel;
class QTimer;
class QPropertyAnimation;
class QVBoxLayout;
class QFrame;

class NotificationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NotificationWidget(QWidget* parent = nullptr);

    void showMessage(const QString& msg);

private:
    QLabel* m_label;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
    QVBoxLayout* m_layout;
    QFrame* m_content;
    QTimer* m_timer;

    void animatedShow();
    void animatedHide();
};
