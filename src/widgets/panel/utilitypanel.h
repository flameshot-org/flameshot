// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

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

signals:
    void mouseEnter();
    void mouseLeave();

public slots:
    void toggle();
    void slotHidePanel();

private:
    void initInternalPanel();

    QPointer<QWidget> m_toolWidget;
    QScrollArea* m_internalPanel;
    QVBoxLayout* m_upLayout;
    QVBoxLayout* m_bottomLayout;
    QVBoxLayout* m_layout;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
    QPushButton* m_hide;
};
