// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Yurii Puchkov at Namecheap & Contributors

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
    explicit SetShortcutDialog(QDialog* parent = nullptr,
                               QString shortcutName = "");
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
