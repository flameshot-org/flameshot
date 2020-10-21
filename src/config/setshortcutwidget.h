// Copyright(c) 2020 Yurii Puchkov at Namecheap & Contributors
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

#ifndef SETSHORTCUTWIDGET_H
#define SETSHORTCUTWIDGET_H

#include <QDialog>
#include <QKeySequence>
#include <QObject>

class QVBoxLayout;

class SetShortcutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SetShortcutDialog(QDialog* parent = nullptr);
    const QKeySequence& shortcut();

public:
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent* event);

signals:

private:
    QVBoxLayout* m_layout;
    QString m_modifier;
    QKeySequence m_ks;
};

#endif // SETSHORTCUTWIDGET_H
