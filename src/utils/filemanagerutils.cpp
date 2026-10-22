// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Dmitrii Shcherbakov & Contributors

#include "filemanagerutils.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#endif

#if defined(Q_OS_WIN)
namespace {
void revealFileNative(const QString& filePath)
{
    // explorer.exe needs a native path and takes the selection target
    // together with the /select switch in a single argument
    const QString nativePath = QDir::toNativeSeparators(filePath);
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            { QStringLiteral("/select,%1").arg(nativePath) });
}
} // namespace
#elif defined(Q_OS_MACOS)
namespace {
void revealFileNative(const QString& filePath)
{
    // "open -R" reveals and selects the file in Finder
    QProcess::startDetached(QStringLiteral("/usr/bin/open"),
                            { QStringLiteral("-R"), filePath });
}
} // namespace
#endif

namespace FileManagerUtils {
void revealFile(const QString& filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return;
    }

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    revealFileNative(fileInfo.absoluteFilePath());
#else
    // org.freedesktop.FileManager1.ShowItems is the desktop-agnostic way
    // to reveal a file, implemented by Nautilus, Nemo, Thunar, Dolphin and
    // others. It is sent asynchronously: a missing implementation must not
    // freeze the caller. When the service is not registered, fall back to
    // opening the containing folder without a selection.
    const QString uri = QUrl::fromLocalFile(fileInfo.absoluteFilePath())
                          .toString(QUrl::FullyEncoded);
    auto bus = QDBusConnection::sessionBus();
    const QString service = QStringLiteral("org.freedesktop.FileManager1");
    if (bus.isConnected() && bus.interface()->isServiceRegistered(service)) {
        QDBusMessage msg = QDBusMessage::createMethodCall(
          service,
          QStringLiteral("/org/freedesktop/FileManager1"),
          service,
          QStringLiteral("ShowItems"));
        msg << QStringList({ uri }) << QString();
        bus.send(msg);
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
    }
#endif
}
}
