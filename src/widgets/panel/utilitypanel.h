// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QPointer>
#include <QWidget>

class QVBoxLayout;
class QPropertyAnimation;
class QScrollArea;
class QPushButton;

class UtilityPanel : public QWidget
{
    Q_OBJECT
public:
    explicit UtilityPanel(QWidget* parent = nullptr);

    QWidget* toolWidget() const;
    void addToolWidget(QWidget* w);
    void clearToolWidget();
    void pushWidget(QWidget* w);
    void hide();
    void show();

public slots:
    void toggle();

private:
    void initInternalPanel();

    QPointer<QWidget> m_toolWidget;
    QScrollArea* m_internalPanel;
    QVBoxLayout* m_upLayout;
    QVBoxLayout* m_bottomLayout;
    QVBoxLayout* m_layout;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
};
