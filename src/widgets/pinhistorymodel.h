// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#pragma once

#include "utils/pinhistoryentry.h"

#include <QAbstractListModel>
#include <QHash>
#include <QPixmap>

class PinHistoryStore;

class PinHistoryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        DismissedAtRole,
        WidthRole,
        HeightRole,
    };

    explicit PinHistoryModel(PinHistoryStore* store, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;

    PinHistoryEntry entryAt(const QModelIndex& index) const;
    QList<PinHistoryEntry> entriesAt(const QModelIndexList& indexes) const;

    void refresh();
    void setNewestFirst(bool newestFirst);
    bool newestFirst() const { return m_newestFirst; }

private:
    QPixmap thumbnailFor(const PinHistoryEntry& entry) const;
    static QString formatTimestamp(const QDateTime& dt);

    PinHistoryStore* m_store;
    QList<PinHistoryEntry> m_rows;
    mutable QHash<QString, QPixmap> m_thumbCache;
    bool m_newestFirst{ true };
};
