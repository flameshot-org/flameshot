// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser <info@ckaiser.com.ar>
// released under the GNU GPL2  <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007 Luca Gugelmann <lucag@student.ethz.ch>
// released under the GNU LGPL  <http://www.gnu.org/licenses/old-licenses/library.txt>

#pragma once

#include <QWidget>
#include <QPointer>

class QVBoxLayout;
class QPropertyAnimation;
class QScrollArea;

class UtilityPanel : public QWidget {
    Q_OBJECT
public:
    explicit UtilityPanel(QWidget *parent = nullptr);

    QWidget* toolWidget() const;
    void addToolWidget(QWidget *w);
    void clearToolWidget();
    void pushWidget(QWidget *w);

signals:
    void mouseEnter();
    void mouseLeave();

public slots:
    void toggle();

private:
    void initInternalPanel();

    QPointer<QWidget> m_toolWidget;
    QScrollArea *m_internalPanel;
    QVBoxLayout *m_upLayout;
    QVBoxLayout *m_layout;
    QPropertyAnimation *m_showAnimation;
    QPropertyAnimation *m_hideAnimation;
};
