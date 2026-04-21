// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#include "pinhistorystore.h"

#include "abstractlogger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcessEnvironment>
#include <QSaveFile>
#include <QUuid>
#include <algorithm>

namespace {

constexpr int kManifestVersion = 1;
const QString kManifestFile = QStringLiteral("manifest.json");
const QString kRecoveredDir = QStringLiteral("recovered");

QString resolveStoragePath()
{
#ifdef Q_OS_WIN
    return QDir::homePath() +
           QStringLiteral("/AppData/Roaming/flameshot/pin-history/");
#else
    const QString cachePath = QProcessEnvironment::systemEnvironment().value(
      QStringLiteral("XDG_CACHE_HOME"),
      QDir::homePath() + QStringLiteral("/.cache"));
    return cachePath + QStringLiteral("/flameshot/pin-history/");
#endif
}

QString makePixmapFileName(const QString& id)
{
    return id + QStringLiteral(".png");
}

} // namespace

PinHistoryStore::PinHistoryStore()
  : m_path(resolveStoragePath())
  , m_manifestPath(m_path + kManifestFile)
{
    if (!ensureDirectory()) {
        m_available = false;
        return;
    }
    m_available = true;

    if (!readManifest()) {
        AbstractLogger::warning() << QStringLiteral(
          "Pin history manifest unreadable, rebuilding from directory scan.");
        rebuildFromScan();
    }
    sortByDismissedDesc();
}

bool PinHistoryStore::ensureDirectory()
{
    QDir dir(m_path);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        AbstractLogger::warning()
          << QStringLiteral("Pin history: unable to create directory at ")
          << m_path;
        return false;
    }
    const QFileInfo info(m_path);
    if (!info.isWritable()) {
        AbstractLogger::warning()
          << QStringLiteral("Pin history: storage path is not writable: ")
          << m_path;
        return false;
    }
    return true;
}

bool PinHistoryStore::insert(const QPixmap& pixmap, PinHistoryEntry entry)
{
    if (!m_available || pixmap.isNull()) {
        return false;
    }
    if (entry.id.isEmpty()) {
        entry.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    entry.pixmapFile = makePixmapFileName(entry.id);
    entry.width = pixmap.width();
    entry.height = pixmap.height();
    if (!entry.dismissedAt.isValid()) {
        entry.dismissedAt = QDateTime::currentDateTimeUtc();
    }
    if (!entry.createdAt.isValid()) {
        entry.createdAt = entry.dismissedAt;
    }

    const QString pixmapPath = m_path + entry.pixmapFile;
    QSaveFile file(pixmapPath);
    if (!file.open(QIODevice::WriteOnly)) {
        AbstractLogger::warning()
          << QStringLiteral("Pin history: failed to open pixmap for write: ")
          << pixmapPath;
        return false;
    }
    if (!pixmap.save(&file, "PNG") || !file.commit()) {
        AbstractLogger::warning()
          << QStringLiteral("Pin history: failed to save pixmap at ")
          << pixmapPath;
        return false;
    }

    m_entries.prepend(entry);
    sortByDismissedDesc();

    if (!writeManifest()) {
        return false;
    }
    return true;
}

PinHistoryEntry PinHistoryStore::entry(const QString& id) const
{
    for (const auto& e : m_entries) {
        if (e.id == id) {
            return e;
        }
    }
    return {};
}

QPixmap PinHistoryStore::loadPixmap(const QString& id) const
{
    const PinHistoryEntry e = entry(id);
    if (!e.isValid()) {
        return {};
    }
    QPixmap p;
    p.load(m_path + e.pixmapFile);
    return p;
}

bool PinHistoryStore::updateThumbFile(const QString& id,
                                      const QString& thumbFile)
{
    for (auto& e : m_entries) {
        if (e.id == id) {
            e.thumbFile = thumbFile;
            return writeManifest();
        }
    }
    return false;
}

bool PinHistoryStore::remove(const QString& id)
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id != id) {
            continue;
        }
        const PinHistoryEntry e = m_entries.takeAt(i);
        QFile::remove(m_path + e.pixmapFile);
        if (!e.thumbFile.isEmpty()) {
            QFile::remove(m_path + e.thumbFile);
        }
        return writeManifest();
    }
    return false;
}

bool PinHistoryStore::removeMany(const QList<QString>& ids)
{
    if (ids.isEmpty()) {
        return true;
    }
    const QSet<QString> idSet(ids.begin(), ids.end());
    QMutableListIterator<PinHistoryEntry> it(m_entries);
    while (it.hasNext()) {
        const PinHistoryEntry& e = it.next();
        if (!idSet.contains(e.id)) {
            continue;
        }
        QFile::remove(m_path + e.pixmapFile);
        if (!e.thumbFile.isEmpty()) {
            QFile::remove(m_path + e.thumbFile);
        }
        it.remove();
    }
    return writeManifest();
}

void PinHistoryStore::clear()
{
    for (const auto& e : m_entries) {
        QFile::remove(m_path + e.pixmapFile);
        if (!e.thumbFile.isEmpty()) {
            QFile::remove(m_path + e.thumbFile);
        }
    }
    m_entries.clear();
    writeManifest();
}

qint64 PinHistoryStore::diskUsage() const
{
    qint64 total = 0;
    for (const auto& e : m_entries) {
        total += QFileInfo(m_path + e.pixmapFile).size();
        if (!e.thumbFile.isEmpty()) {
            total += QFileInfo(m_path + e.thumbFile).size();
        }
    }
    total += QFileInfo(m_manifestPath).size();
    return total;
}

void PinHistoryStore::applyRetention(int maxEntries, int maxAgeDays)
{
    bool dirty = false;
    if (maxAgeDays > 0) {
        const QDateTime cutoff =
          QDateTime::currentDateTimeUtc().addDays(-maxAgeDays);
        QMutableListIterator<PinHistoryEntry> it(m_entries);
        while (it.hasNext()) {
            const PinHistoryEntry& e = it.next();
            if (e.dismissedAt.isValid() && e.dismissedAt < cutoff) {
                QFile::remove(m_path + e.pixmapFile);
                if (!e.thumbFile.isEmpty()) {
                    QFile::remove(m_path + e.thumbFile);
                }
                it.remove();
                dirty = true;
            }
        }
    }
    if (maxEntries > 0 && m_entries.size() > maxEntries) {
        sortByDismissedDesc();
        while (m_entries.size() > maxEntries) {
            const PinHistoryEntry e = m_entries.takeLast();
            QFile::remove(m_path + e.pixmapFile);
            if (!e.thumbFile.isEmpty()) {
                QFile::remove(m_path + e.thumbFile);
            }
            dirty = true;
        }
    }
    if (dirty) {
        writeManifest();
    }
}

bool PinHistoryStore::writeManifest()
{
    if (!m_available) {
        return false;
    }
    QJsonArray arr;
    for (const auto& e : m_entries) {
        arr.append(e.toJson());
    }
    QJsonObject root;
    root.insert(QStringLiteral("version"), kManifestVersion);
    root.insert(QStringLiteral("entries"), arr);

    QSaveFile file(m_manifestPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        AbstractLogger::warning()
          << QStringLiteral("Pin history: failed to open manifest for write.");
        return false;
    }
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size() || !file.commit()) {
        AbstractLogger::warning()
          << QStringLiteral("Pin history: failed to commit manifest.");
        return false;
    }
    return true;
}

bool PinHistoryStore::readManifest()
{
    QFile file(m_manifestPath);
    if (!file.exists()) {
        m_entries.clear();
        return true;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }
    const QJsonArray arr =
      doc.object().value(QStringLiteral("entries")).toArray();
    m_entries.clear();
    m_entries.reserve(arr.size());
    for (const auto& v : arr) {
        if (!v.isObject()) {
            continue;
        }
        PinHistoryEntry e = PinHistoryEntry::fromJson(v.toObject());
        if (!e.isValid()) {
            continue;
        }
        if (!QFileInfo::exists(m_path + e.pixmapFile)) {
            continue; // stale manifest entry, drop silently
        }
        m_entries.append(e);
    }
    return true;
}

void PinHistoryStore::rebuildFromScan()
{
    m_entries.clear();
    QDir dir(m_path);
    const QFileInfoList pngs = dir.entryInfoList(
      QStringList() << QStringLiteral("*.png"), QDir::Files, QDir::Time);

    const QString recoveredPath = m_path + kRecoveredDir + QStringLiteral("/");
    bool recoveredDirCreated = false;

    for (const QFileInfo& info : pngs) {
        const QString base = info.completeBaseName();
        if (base.endsWith(QStringLiteral("-thumb"))) {
            continue; // handled via main entry, or orphaned
        }
        const QUuid uuid = QUuid::fromString(base);
        if (uuid.isNull()) {
            // Unknown file — quarantine rather than delete.
            if (!recoveredDirCreated) {
                QDir().mkpath(recoveredPath);
                recoveredDirCreated = true;
            }
            QFile::rename(info.absoluteFilePath(),
                          recoveredPath + info.fileName());
            continue;
        }
        PinHistoryEntry e;
        e.id = uuid.toString(QUuid::WithoutBraces);
        e.pixmapFile = info.fileName();
        e.createdAt = info.birthTime().toUTC();
        if (!e.createdAt.isValid()) {
            e.createdAt = info.lastModified().toUTC();
        }
        e.dismissedAt = info.lastModified().toUTC();

        const QString candidateThumb = base + QStringLiteral("-thumb.png");
        if (QFileInfo::exists(m_path + candidateThumb)) {
            e.thumbFile = candidateThumb;
        }

        QPixmap probe(info.absoluteFilePath());
        if (!probe.isNull()) {
            e.width = probe.width();
            e.height = probe.height();
        }
        m_entries.append(e);
    }
    writeManifest();
}

void PinHistoryStore::sortByDismissedDesc()
{
    std::sort(m_entries.begin(),
              m_entries.end(),
              [](const PinHistoryEntry& a, const PinHistoryEntry& b) {
                  return a.dismissedAt > b.dismissedAt;
              });
}
