// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#include "pinhistorymanager.h"

#include "abstractlogger.h"
#include "confighandler.h"
#include "core/flameshotdaemon.h"

#include <QApplication>
#include <QDateTime>
#include <QPixmap>
#include <QScreen>

PinHistoryManager::PinHistoryManager(QObject* parent)
  : QObject(parent)
{
    if (!m_store.isAvailable()) {
        AbstractLogger::warning() << QStringLiteral(
          "Pin history disabled for this session: storage path unavailable.");
        return;
    }
    applyRetention();
}

bool PinHistoryManager::isEnabled() const
{
    if (!m_store.isAvailable()) {
        return false;
    }
    return ConfigHandler().pinHistoryEnabled();
}

void PinHistoryManager::applyRetention()
{
    ConfigHandler cfg;
    m_store.applyRetention(cfg.pinHistoryMaxEntries(),
                           cfg.pinHistoryMaxAgeDays());
}

void PinHistoryManager::recordDismissal(const QPixmap& pixmap,
                                        const QRect& geometry,
                                        qreal zoom,
                                        qreal opacity,
                                        const QString& screenName)
{
    if (!isEnabled() || pixmap.isNull()) {
        return;
    }
    PinHistoryEntry entry;
    entry.geometry = geometry;
    entry.screenName = screenName;
    entry.zoom = zoom;
    entry.opacity = opacity;
    entry.dismissedAt = QDateTime::currentDateTimeUtc();
    entry.createdAt = entry.dismissedAt;

    if (!m_store.insert(pixmap, entry)) {
        if (FlameshotDaemon* daemon = FlameshotDaemon::instance()) {
            daemon->sendTrayNotification(
              tr("Failed to save pin to history. Check available disk space."),
              QStringLiteral("Flameshot"));
        }
        return;
    }
    applyRetention();
    emit changed();
}

bool PinHistoryManager::restore(const QString& id)
{
    const PinHistoryEntry entry = m_store.entry(id);
    if (!entry.isValid()) {
        return false;
    }
    const QPixmap pixmap = m_store.loadPixmap(id);
    if (pixmap.isNull()) {
        return false;
    }

    QRect target = entry.geometry;
    QScreen* targetScreen = nullptr;
    if (!entry.screenName.isEmpty()) {
        for (QScreen* s : QGuiApplication::screens()) {
            if (s->name() == entry.screenName) {
                targetScreen = s;
                break;
            }
        }
    }
    if (!targetScreen) {
        targetScreen = QGuiApplication::primaryScreen();
        if (targetScreen) {
            const QRect screenGeom = targetScreen->availableGeometry();
            target.moveCenter(screenGeom.center());
        }
    } else {
        target = target.intersected(targetScreen->geometry());
        if (target.isEmpty()) {
            target = entry.geometry;
            target.moveCenter(targetScreen->availableGeometry().center());
        }
    }

    FlameshotDaemon* daemon = FlameshotDaemon::instance();
    if (!daemon) {
        return false;
    }
    daemon->attachPin(pixmap, target, entry.zoom, entry.opacity);
    return true;
}
