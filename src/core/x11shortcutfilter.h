// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QObject>
#include <QThread>
#include <atomic>

class X11ShortcutWorker : public QObject
{
    Q_OBJECT
public:
    explicit X11ShortcutWorker(QObject* parent = nullptr);

public slots:
    void run();
    void stop();

signals:
    void printPressed();

private:
    std::atomic<bool> m_running;
};

class X11ShortcutFilter : public QObject
{
    Q_OBJECT
public:
    explicit X11ShortcutFilter(QObject* parent = nullptr);
    ~X11ShortcutFilter() override;

signals:
    void printPressed();

private:
    QThread m_thread;
    X11ShortcutWorker* m_worker;
};
