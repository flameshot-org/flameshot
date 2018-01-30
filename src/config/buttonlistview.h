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

#pragma once

#include "src/capture/widgets/capturebutton.h"
#include <QListWidget>

class ButtonListView : public QListWidget {
public:
    ButtonListView(QWidget *parent= nullptr);

public slots:
    void selectAll();
    void updateComponents();

private slots:
    void reverseItemCheck(QListWidgetItem *);

protected:
    void initButtonList();

private:
    QVector<CaptureButton::ButtonType> m_listButtons;
    QMap<QString, CaptureButton::ButtonType> m_buttonTypeByName;

    void updateActiveButtons(QListWidgetItem *);

};
