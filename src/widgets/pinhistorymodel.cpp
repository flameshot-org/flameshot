// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#include "pinhistorymodel.h"

#include "utils/pinhistorystore.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QLocale>
#include <algorithm>

namespace {
constexpr int kThumbWidth = 164;
constexpr int kThumbHeight = 120;
} // namespace

PinHistoryModel::PinHistoryModel(PinHistoryStore* store, QObject* parent)
  : QAbstractListModel(parent)
  , m_store(store)
{
    refresh();
}

int PinHistoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rows.size();
}

QVariant PinHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }
    const PinHistoryEntry& e = m_rows[index.row()];

    switch (role) {
        case Qt::DisplayRole:
            return QCoreApplication::translate("PinHistoryModel", "%1\n%2 × %3")
              .arg(formatTimestamp(e.dismissedAt))
              .arg(e.width)
              .arg(e.height);
        case Qt::DecorationRole:
            return thumbnailFor(e);
        case Qt::ToolTipRole:
            return QCoreApplication::translate("PinHistoryModel",
                                               "Dismissed %1\n%2 × %3 px")
              .arg(QLocale::system().toString(e.dismissedAt.toLocalTime(),
                                              QLocale::LongFormat))
              .arg(e.width)
              .arg(e.height);
        case IdRole:
            return e.id;
        case DismissedAtRole:
            return e.dismissedAt;
        case WidthRole:
            return e.width;
        case HeightRole:
            return e.height;
        default:
            return {};
    }
}

PinHistoryEntry PinHistoryModel::entryAt(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }
    return m_rows[index.row()];
}

QList<PinHistoryEntry> PinHistoryModel::entriesAt(
  const QModelIndexList& indexes) const
{
    QList<PinHistoryEntry> out;
    out.reserve(indexes.size());
    for (const QModelIndex& i : indexes) {
        const PinHistoryEntry e = entryAt(i);
        if (e.isValid()) {
            out.append(e);
        }
    }
    return out;
}

void PinHistoryModel::refresh()
{
    beginResetModel();
    m_rows = m_store ? m_store->entries() : QList<PinHistoryEntry>{};
    std::sort(m_rows.begin(),
              m_rows.end(),
              [this](const PinHistoryEntry& a, const PinHistoryEntry& b) {
                  return m_newestFirst ? a.dismissedAt > b.dismissedAt
                                       : a.dismissedAt < b.dismissedAt;
              });
    // Cleanup stale thumbnail cache entries.
    QSet<QString> liveIds;
    liveIds.reserve(m_rows.size());
    for (const auto& e : m_rows) {
        liveIds.insert(e.id);
    }
    for (auto it = m_thumbCache.begin(); it != m_thumbCache.end();) {
        if (!liveIds.contains(it.key())) {
            it = m_thumbCache.erase(it);
        } else {
            ++it;
        }
    }
    endResetModel();
}

void PinHistoryModel::setNewestFirst(bool newestFirst)
{
    if (m_newestFirst == newestFirst) {
        return;
    }
    m_newestFirst = newestFirst;
    refresh();
}

QPixmap PinHistoryModel::thumbnailFor(const PinHistoryEntry& entry) const
{
    if (!m_store) {
        return {};
    }
    auto it = m_thumbCache.find(entry.id);
    if (it != m_thumbCache.end()) {
        return it.value();
    }
    const QPixmap full = m_store->loadPixmap(entry.id);
    if (full.isNull()) {
        return {};
    }
    const QPixmap thumb = full.scaled(
      kThumbWidth, kThumbHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_thumbCache.insert(entry.id, thumb);
    return thumb;
}

QString PinHistoryModel::formatTimestamp(const QDateTime& dt)
{
    if (!dt.isValid()) {
        return {};
    }
    const QDateTime local = dt.toLocalTime();
    const QDate today = QDate::currentDate();
    const QLocale locale = QLocale::system();
    if (local.date() == today) {
        return QCoreApplication::translate("PinHistoryModel", "Today %1")
          .arg(locale.toString(local.time(), QLocale::ShortFormat));
    }
    if (local.date() == today.addDays(-1)) {
        return QCoreApplication::translate("PinHistoryModel", "Yesterday %1")
          .arg(locale.toString(local.time(), QLocale::ShortFormat));
    }
    return locale.toString(local, QLocale::ShortFormat);
}
