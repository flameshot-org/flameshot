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

#include "strftimechooserwidget.h"
#include <QMap>
#include <QGridLayout>
#include <QPushButton>

StrftimeChooserWidget::StrftimeChooserWidget(QWidget *parent) : QWidget(parent) {
    QGridLayout *layout = new QGridLayout(this);
    auto k = m_buttonData.keys();
    int middle = k.length()/2;
    // add the buttons in 2 columns (they need to be even)
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < middle; j++) {
            QString key = k.last();
            k.pop_back();
            QString variable = m_buttonData.value(key);
            QPushButton *button = new QPushButton(this);
            button->setText(tr(key.toStdString().data()));
            button->setToolTip(variable);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            button->setMinimumHeight(25);
            layout->addWidget(button, j, i);
            connect(button, &QPushButton::clicked,
                    this, [variable, this](){Q_EMIT variableEmitted(variable);});
        }
    }
    setLayout(layout);
}

QMap<QString, QString> StrftimeChooserWidget::m_buttonData {
    { QT_TR_NOOP("Century (00-99)"),        "%C"},
    { QT_TR_NOOP("Year (00-99)"),           "%y"},
    { QT_TR_NOOP("Year (2000)"),            "%Y"},
    { QT_TR_NOOP("Month Name (jan)"),       "%b"},
    { QT_TR_NOOP("Month Name (january)"),   "%B"},
    { QT_TR_NOOP("Month (01-12)"),          "%m"},
    { QT_TR_NOOP("Week Day (1-7)"),         "%u"},
    { QT_TR_NOOP("Week (01-53)"),           "%V"},
    { QT_TR_NOOP("Day Name (mon)"),         "%a"},
    { QT_TR_NOOP("Day Name (monday)"),      "%A"},
    { QT_TR_NOOP("Day (01-31)"),            "%d"},
    { QT_TR_NOOP("Day of Month (1-31)"),    "%e"},
    { QT_TR_NOOP("Day (001-366)"),          "%j"},
    { QT_TR_NOOP("Time (%H:%M:%S)"),        "%T"},
    { QT_TR_NOOP("Time (%H:%M)"),           "%R"},
    { QT_TR_NOOP("Hour (00-23)"),           "%H"},
    { QT_TR_NOOP("Hour (01-12)"),           "%I"},
    { QT_TR_NOOP("Minute (00-59)"),         "%M"},
    { QT_TR_NOOP("Second (00-59)"),         "%S"},
    { QT_TR_NOOP("Full Date (%m/%d/%y)"),   "%D"},
    { QT_TR_NOOP("Full Date (%Y-%m-%d)"),   "%F"},
};
