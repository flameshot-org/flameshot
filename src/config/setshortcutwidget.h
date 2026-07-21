// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2020 Yurii Puchkov at Namecheap & Contributors

#pragma once

#include <QDialog>
#include <QKeySequence>
#include <QObject>

class QKeyEvent;
class QVBoxLayout;

class SetShortcutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SetShortcutDialog(QDialog* parent = nullptr);
    ~SetShortcutDialog() override;
    const QKeySequence& shortcut();

public:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent* event) override;
#if defined(Q_OS_WIN)
    bool nativeEvent(const QByteArray& eventType,
                     void* message,
                     qintptr* result) override;
#endif

private slots:
    void accept() override;
    void reject() override;

private:
    void startCapture();
    void stopCapture();
    void updateShortcutFromKeyEvent(QKeyEvent* event);

    QVBoxLayout* m_layout;
    QKeySequence m_ks;
#if defined(Q_OS_WIN)
    bool m_printScreenHotkeyRegistered = false;
#endif
};
