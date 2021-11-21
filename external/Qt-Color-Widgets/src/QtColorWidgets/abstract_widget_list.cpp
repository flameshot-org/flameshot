/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "QtColorWidgets/abstract_widget_list.hpp"
#include <QToolButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>

class AbstractWidgetList::Private
{
public:
    QList<QWidget*> widgets;
    QSignalMapper mapper_up;
    QSignalMapper mapper_down;
    QSignalMapper mapper_remove;
    QTableWidget *table;
};

AbstractWidgetList::AbstractWidgetList(QWidget *parent) :
    QWidget(parent), p(new Private)
{
    connect(&p->mapper_up,SIGNAL(mapped(QWidget*)),SLOT(up_clicked(QWidget*)));
    connect(&p->mapper_down,SIGNAL(mapped(QWidget*)),SLOT(down_clicked(QWidget*)));
    connect(&p->mapper_remove,SIGNAL(mapped(QWidget*)),SLOT(remove_clicked(QWidget*)));


    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    p->table = new QTableWidget(this);
    verticalLayout->addWidget(p->table);


    p->table->insertColumn(0);
    p->table->insertColumn(1);
    p->table->insertColumn(2);
    p->table->insertColumn(3);

    p->table->setColumnWidth(0,128);
    p->table->setColumnWidth(1,24);
    p->table->setColumnWidth(2,24);
    p->table->setColumnWidth(3,24);

    p->table->horizontalHeader()->hide();
    p->table->verticalHeader()->hide();
    p->table->setShowGrid(false);

    QPushButton* add_button = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")),
                                              tr("Add New"));

    verticalLayout->addWidget(add_button);
    connect(add_button,&QAbstractButton::clicked,this, &AbstractWidgetList::append);

}

AbstractWidgetList::~AbstractWidgetList()
{
    delete p;
}

int AbstractWidgetList::count() const
{
    return p->widgets.size();
}

void AbstractWidgetList::setRowHeight(int row, int height)
{
    p->table->setRowHeight(row,height);
}

void AbstractWidgetList::clear()
{
    p->widgets.clear();
    while(p->table->rowCount() > 0)
        p->table->removeRow(0);
}


void AbstractWidgetList::remove(int i)
{
    if ( isValidRow(i) )
    {
        p->widgets.removeAt(i);
        p->table->removeRow(i);
        if ( i == 0 && !p->widgets.isEmpty() )
            p->table->cellWidget(0,1)->setEnabled(false);
        else if ( i != 0 && i == count() )
            p->table->cellWidget(count()-1,2)->setEnabled(false);

        Q_EMIT removed(i);
    }
}


void AbstractWidgetList::appendWidget(QWidget *w)
{
    int row = count();
    p->table->insertRow(row);

    QWidget* b_up = create_button(w,&p->mapper_up,QStringLiteral("go-up"),tr("Move Up"));
    QWidget* b_down = create_button(w,&p->mapper_down,QStringLiteral("go-down"),tr("Move Down"));
    QWidget* b_remove = create_button(w,&p->mapper_remove,QStringLiteral("list-remove"),tr("Remove"));
    if ( row == 0 )
        b_up->setEnabled(false);
    else
        p->table->cellWidget(row-1,2)->setEnabled(true);
    b_down->setEnabled(false);

    p->table->setCellWidget(row,0,w);
    p->table->setCellWidget(row,1,b_up);
    p->table->setCellWidget(row,2,b_down);
    p->table->setCellWidget(row,3,b_remove);

    p->widgets.push_back(w);
}

QWidget *AbstractWidgetList::widget(int i)
{
    if ( isValidRow(i) )
        return p->widgets[i];
    return 0;
}


QWidget *AbstractWidgetList::create_button(QWidget *data, QSignalMapper *mapper,
                                             QString icon_name,
                                             QString text, QString tooltip) const
{

    QToolButton* btn = new QToolButton;
    btn->setIcon(QIcon::fromTheme(icon_name));
    btn->setText(text);
    btn->setToolTip(tooltip.isNull() ? btn->text() : tooltip );
    connect(btn,SIGNAL(clicked()),mapper,SLOT(map()));
    mapper->setMapping(btn,data);
    return btn;
}

void AbstractWidgetList::remove_clicked(QWidget *w)
{
    int row = p->widgets.indexOf(w);
    remove(row);
}

void AbstractWidgetList::up_clicked(QWidget *w)
{
    int row = p->widgets.indexOf(w);
    if ( row > 0 )
        swap(row,row-1);
}

void AbstractWidgetList::down_clicked(QWidget *w)
{
    int row = p->widgets.indexOf(w);
    if ( row+1 < count() )
        swap(row,row+1);
}
