// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#pragma once

#include "pinhistorystore.h"

#include <QObject>

class PinHistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit PinHistoryManager(QObject* parent = nullptr);

    bool isEnabled() const;
    PinHistoryStore* store() { return &m_store; }
    const PinHistoryStore* store() const { return &m_store; }

    QList<PinHistoryEntry> entries() const { return m_store.entries(); }
    void applyRetention();

public slots:
    void recordDismissal(const QPixmap& pixmap,
                         const QRect& geometry,
                         qreal zoom,
                         qreal opacity,
                         const QString& screenName);
    bool restore(const QString& id);

signals:
    void changed();

private:
    PinHistoryStore m_store;
};
