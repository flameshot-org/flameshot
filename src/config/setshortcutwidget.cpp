// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Yurii Puchkov at Namecheap & Contributors

#include "setshortcutwidget.h"
#include "src/utils/globalvalues.h"
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QPixmap>

SetShortcutDialog::SetShortcutDialog(QDialog* parent, QString shortcutName)
  : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("Set Shortcut"));
    m_ks = QKeySequence();

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignHCenter);

    auto* infoTop = new QLabel(tr("Enter new shortcut to change "));
    infoTop->setMargin(10);
    infoTop->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(infoTop);

    auto* infoIcon = new QLabel();
    infoIcon->setAlignment(Qt::AlignCenter);
    infoIcon->setPixmap(QPixmap(":/img/app/keyboard.svg"));
    m_layout->addWidget(infoIcon);

    m_layout->addWidget(infoIcon);

    QString msg = "";
#if defined(Q_OS_MAC)
    msg = tr(
      "Press Esc to cancel or ⌘+Backspace to disable the keyboard shortcut.");
#else
    msg =
      tr("Press Esc to cancel or Backspace to disable the keyboard shortcut.");
#endif
    if (shortcutName == "TAKE_SCREENSHOT" ||
        shortcutName == "SCREENSHOT_HISTORY") {
        msg +=
          "\n" + tr("Flameshot must be restarted for changes to take effect.");
    }
    auto* infoBottom = new QLabel(msg);
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
    if (ke->modifiers() & Qt::ShiftModifier) {
        m_modifier += "Shift+";
    }
    if (ke->modifiers() & Qt::ControlModifier) {
        m_modifier += "Ctrl+";
    }
    if (ke->modifiers() & Qt::AltModifier) {
        m_modifier += "Alt+";
    }
    if (ke->modifiers() & Qt::MetaModifier) {
        m_modifier += "Meta+";
    }

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
