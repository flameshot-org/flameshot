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

#include "shortcutswidget.h"
#include "setshortcutwidget.h"
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVector>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QCursor>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#endif

ShortcutsWidget::ShortcutsWidget(QWidget* parent)
  : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowTitle(tr("Hot Keys"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignHCenter);

    m_shortcuts = m_config.shortcuts();
    initInfoTable();
    show();
}

const QVector<QStringList>& ShortcutsWidget::shortcuts()
{
    return m_shortcuts;
}

void ShortcutsWidget::initInfoTable()
{
    m_table = new QTableWidget(this);
    m_table->setToolTip(tr("Available shortcuts in the screen capture mode."));

    m_layout->addWidget(m_table);

    m_table->setColumnCount(2);
    m_table->setRowCount(m_shortcuts.size());
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->verticalHeader()->hide();

    // header creation
    QStringList names;
    names << tr("Description") << tr("Key");
    m_table->setHorizontalHeaderLabels(names);
    connect(m_table,
            SIGNAL(cellClicked(int, int)),
            this,
            SLOT(slotShortcutCellClicked(int, int)));

    // add content
    for (int i = 0; i < shortcuts().size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(m_shortcuts.at(i).at(1)));

        QTableWidgetItem* item = new QTableWidgetItem(m_shortcuts.at(i).at(2));
        item->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 1, item);

        if (m_shortcuts.at(i).at(0).isEmpty()) {
            QFont font;
            font.setBold(true);
            item->setFont(font);
            item->setFlags(item->flags() ^ Qt::ItemIsEnabled);
            m_table->item(i, 1)->setFont(font);
        }
    }

    // Read-only table items
    for (int x = 0; x < m_table->rowCount(); ++x) {
        for (int y = 0; y < m_table->columnCount(); ++y) {
            QTableWidgetItem* item = m_table->item(x, y);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        }
    }

    // adjust size
    m_table->resizeColumnsToContents();
    m_table->resizeRowsToContents();
    m_table->setMinimumWidth(400);
    m_table->setMaximumWidth(600);

    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSizePolicy(QSizePolicy::Expanding,
                                               QSizePolicy::Expanding);
}

void ShortcutsWidget::slotShortcutCellClicked(int row, int col)
{
    if (col == 1) {
        // Ignore non-changable shortcuts
        if (Qt::ItemIsEnabled !=
            (Qt::ItemIsEnabled & m_table->item(row, col)->flags())) {
            return;
        }

        SetShortcutDialog* setShortcutDialog = new SetShortcutDialog();
        if (0 != setShortcutDialog->exec()) {
            QString shortcutName = m_shortcuts.at(row).at(0);
            QKeySequence shortcutValue = setShortcutDialog->shortcut();

            // set no shortcut is Backspace
            if (shortcutValue == QKeySequence(Qt::Key_Backspace)) {
                shortcutValue = QKeySequence("");
            }

            if (m_config.setShortcut(shortcutName, shortcutValue.toString())) {
                QTableWidgetItem* item =
                  new QTableWidgetItem(shortcutValue.toString());
                item->setTextAlignment(Qt::AlignCenter);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                m_table->setItem(row, col, item);
            }
        }
        delete setShortcutDialog;
    }
}
