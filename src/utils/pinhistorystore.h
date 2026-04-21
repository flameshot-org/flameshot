// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#pragma once

#include "pinhistoryentry.h"

#include <QList>
#include <QPixmap>
#include <QString>

class PinHistoryStore
{
public:
    PinHistoryStore();

    bool isAvailable() const { return m_available; }
    const QString& path() const { return m_path; }

    bool insert(const QPixmap& pixmap, PinHistoryEntry entry);
    QList<PinHistoryEntry> entries() const { return m_entries; }
    PinHistoryEntry entry(const QString& id) const;

    QPixmap loadPixmap(const QString& id) const;
    bool updateThumbFile(const QString& id, const QString& thumbFile);

    bool remove(const QString& id);
    bool removeMany(const QList<QString>& ids);
    void clear();

    qint64 diskUsage() const;

    void applyRetention(int maxEntries, int maxAgeDays);

private:
    bool ensureDirectory();
    bool writeManifest();
    bool readManifest();
    void rebuildFromScan();
    void sortByDismissedDesc();

    QString m_path;
    QString m_manifestPath;
    QList<PinHistoryEntry> m_entries;
    bool m_available{ false };
};
