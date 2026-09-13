// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "actionplugin.h"

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QTimer>

class QPixmap;

class ActionPluginRunner : public QObject
{
    Q_OBJECT
public:
    static ActionPluginRunner* run(const ActionPlugin& plugin,
                                   const QPixmap& capture);

signals:
    void finished(bool success);

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processFailed();
    void processTimedOut();

private:
    explicit ActionPluginRunner(const ActionPlugin& plugin,
                                const QPixmap& capture);

    void start();
    bool collectOutput();
    void finishWithError(const QString& message);
    void finishSuccessfully(const QByteArray& standardOutput);
    bool handleJsonResult(const QByteArray& standardOutput);

    ActionPlugin m_plugin;
    QByteArray m_png;
    QByteArray m_standardOutput;
    QByteArray m_standardError;
    QProcess m_process;
    QTimer m_timeout;
    bool m_finished = false;
};
