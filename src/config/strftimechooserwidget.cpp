// Copyright 2017 Alejandro Sirgo Rica
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

#include "strftimechooserwidget.h"
#include <QMap>
#include <QGridLayout>
#include <QPushButton>

StrftimeChooserWidget::StrftimeChooserWidget(QWidget *parent) : QWidget(parent) {
    QGridLayout *layout = new QGridLayout(this);
    auto k = m_buttonData.keys();
    int middle = k.length()/2;
    // add the buttons in 2 columns (they need to be even)
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < middle; j++) {
            QString key = k.last();
            k.pop_back();
            QString variable = m_buttonData.value(key);
            QPushButton *button = new QPushButton(this);
            button->setText(key);
            button->setToolTip(variable);
            button->setFixedHeight(30);
            layout->addWidget(button, j, i);
            connect(button, &QPushButton::clicked,
                    this, [variable, this](){Q_EMIT variableEmitted(variable);});
        }
    }
    setLayout(layout);
}

QMap<QString, QString> StrftimeChooserWidget::m_buttonData {
    { "Century (00-99)",        "%C"},
    { "Year (00-99)",           "%y"},
    { "Year (2000)",            "%Y"},
    { "Month Name (jan)",       "%b"},
    { "Month Name (january)",   "%B"},
    { "Month (01-12)",          "%m"},
    { "Week Day (1-7)",         "%u"},
    { "week (01-53)",           "%V"},
    { "Day Name (mon)",         "%a"},
    { "Day Name (monday)",      "%A"},
    { "Day (01-31)",            "%d"},
    { "Day of Month (1-31)",    "%e"},
    { "Day (001-366)",          "%j"},
    { "Time (%H:%M:%S)",        "%T"},
    { "Time (%H:%M)",           "%R"},
    { "Hour (00-23)",           "%H"},
    { "Hour (01-12)",           "%I"},
    { "Minute (00-59)",         "%M"},
    { "Second (00-59)",         "%S"},
    { "Full Date (%m/%d/%y)",   "%D"},
    { "Full Date (%Y-%m-%d)",   "%F"},
};
