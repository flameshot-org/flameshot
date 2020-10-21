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

#include "setshortcutwidget.h"
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QPixmap>

SetShortcutDialog::SetShortcutDialog(QDialog* parent)
  : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowTitle(tr("Set Shortcut"));
    m_ks = QKeySequence();

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignHCenter);

    QLabel* infoTop = new QLabel(tr("Enter new shortcut to change "));
    infoTop->setMargin(10);
    infoTop->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(infoTop);

    QLabel* infoIcon = new QLabel();
    infoIcon->setAlignment(Qt::AlignCenter);
    infoIcon->setPixmap(QPixmap(":/img/app/keyboard.svg"));
    m_layout->addWidget(infoIcon);

    m_layout->addWidget(infoIcon);

    QLabel* infoBottom = new QLabel(
      tr("Press Esc to cancel or Backspace to disable the keyboard shortcut."));
    infoBottom->setMargin(10);
    infoBottom->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(infoBottom);
}

const QKeySequence& SetShortcutDialog::shortcut()
{
    return m_ks;
}

void SetShortcutDialog::keyPressEvent(QKeyEvent* ke)
{
    if (ke->modifiers() & Qt::ShiftModifier)
        m_modifier += "Shift+";
    if (ke->modifiers() & Qt::ControlModifier)
        m_modifier += "Ctrl+";
    if (ke->modifiers() & Qt::AltModifier)
        m_modifier += "Alt+";
    if (ke->modifiers() & Qt::MetaModifier)
        m_modifier += "Meta+";

    QString key = QKeySequence(ke->key()).toString();
    m_ks = QKeySequence(m_modifier + key);
}

void SetShortcutDialog::keyReleaseEvent(QKeyEvent* event)
{
    if (m_ks == QKeySequence(Qt::Key_Escape)) {
        reject();
    }
    accept();
}
