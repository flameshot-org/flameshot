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
                               const QString& shortcutName = "");
    const QKeySequence& shortcut();

public:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void accept() override;
    void reject() override;

private:
    void startCapture();

    QVBoxLayout* m_layout;
    QString m_modifier;
    QKeySequence m_ks;
};

#endif // SETSHORTCUTWIDGET_H
