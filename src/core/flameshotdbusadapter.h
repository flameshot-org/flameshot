// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/core/controller.h"
#include <QtDBus/QDBusAbstractAdaptor>

class FlameshotDBusAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.flameshot.Flameshot")

public:
    explicit FlameshotDBusAdapter(QObject* parent = nullptr);
    virtual ~FlameshotDBusAdapter();

signals:
    void captureTaken(uint id, QByteArray rawImage, QRect selection);
    void captureFailed(uint id);
    void captureSaved(uint id, QString savePath);

public slots:
    Q_NOREPLY void graphicCapture(QString path, int delay, uint id);
    Q_NOREPLY void fullScreen(QString path,
                              bool toClipboard,
                              int delay,
                              uint id);
    Q_NOREPLY void captureScreen(int number,
                                 QString path,
                                 bool toClipboard,
                                 int delay,
                                 uint id);
    Q_NOREPLY void openLauncher();
    Q_NOREPLY void openConfig();
    Q_NOREPLY void trayIconEnabled(bool enabled);
    Q_NOREPLY void autostartEnabled(bool enabled);

private slots:
    void handleCaptureTaken(uint id, const QPixmap& p, QRect selection);
};
