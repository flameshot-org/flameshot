// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>

class GlobalShortcutFilter
  : public QObject
  , public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit GlobalShortcutFilter(QObject* parent = nullptr);
    ~GlobalShortcutFilter() override;

    bool nativeEventFilter(const QByteArray& eventType,
                           void* message,
                           qintptr* result) override;

private:
    bool m_printScreenRegistered{ false };
    bool m_shiftPrintScreenRegistered{ false };
};
