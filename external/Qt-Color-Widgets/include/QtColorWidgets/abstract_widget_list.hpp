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
#ifndef ABSTRACT_WIDGET_LIST_HPP
#define ABSTRACT_WIDGET_LIST_HPP

#include "colorwidgets_global.hpp"

#include <QSignalMapper>
#include <QTableWidget>

class QCP_EXPORT AbstractWidgetList : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractWidgetList(QWidget *parent = 0);
    ~AbstractWidgetList();
    
    /**
     *  \brief Get the number of items
     */
    int count() const;

    /**
     *  \brief Swap row a and row b
     */
    virtual void swap(int a, int b) = 0;


    /// Whether the given row index is valid
    bool isValidRow(int i) const { return i >= 0 && i < count(); }

    void setRowHeight(int row, int height);


public Q_SLOTS:
    /**
     *  \brief Remove row i
     */
    void remove(int i);

    /**
     *  \brief append a default row
     */
    virtual void append() = 0;

Q_SIGNALS:
    void removed(int i);

protected:

    /**
     *  \brief Create a new row with the given widget
     *
     *  Must be caled by implementations of append()
     */
    void appendWidget(QWidget* w);

    /**
     *  \brief get the widget found at the given row
     */
    QWidget* widget(int i);


    /**
     *  \brief get the widget found at the given row
     */
    template<class T>
    T* widget_cast(int i) { return qobject_cast<T*>(widget(i)); }

    /**
     *  \brief clear all rows without emitting signals
     *
     *  May be useful when implementation want to set all values at once
     */
    void clear();

private Q_SLOTS:
    void remove_clicked(QWidget* w);
    void up_clicked(QWidget* w);
    void down_clicked(QWidget* w);

private:
    class Private;
    Private * const p;

    QWidget* create_button(QWidget* data, QSignalMapper*mapper,
                           QString icon_name, QString text,
                           QString tooltip = QString()) const;
};

#endif // ABSTRACT_WIDGET_LIST_HPP
