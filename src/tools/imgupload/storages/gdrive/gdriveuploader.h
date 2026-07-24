// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#pragma once

#include "tools/imgupload/storages/imguploaderbase.h"

#include <QByteArray>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QUrl;
class QJsonObject;

/**
 * Google Drive upload backend (R5-R14). Uploads the capture into a single
 * fixed-name folder in the user's own My Drive, applies the chosen sharing
 * visibility, returns the shareable link, and supports delete.
 *
 * Each Drive/OAuth request connects its own reply signals (KTD10) rather than a
 * single shared finished() hookup, because the upload is a multi-step sequence
 * (token -> folder -> resumable upload -> permission -> link).
 */
class GDriveUploader : public ImgUploaderBase
{
    Q_OBJECT
public:
    explicit GDriveUploader(const QPixmap& capture, QWidget* parent = nullptr);
    ~GDriveUploader() override;

    void upload() override;
    void deleteImage(const QString& fileName,
                     const QString& deleteToken) override;

    // Per-upload sharing selection (set by U6 before upload()). Defaults come
    // from the configured default visibility.
    void setVisibility(const QString& visibility);
    void setRecipients(const QStringList& recipients);

private:
    // Acquire an access token via the shared OAuth service and resolve through
    // exactly one callback, owning the attach/connect/disconnect/detach
    // bookkeeping once for both the upload and delete flows.
    void withAccessToken(std::function<void(const QString&)> onReady,
                         std::function<void()> onCanceled,
                         std::function<void(const QString&)> onFailed);
    void ensureFolder();
    void findOrCreateFolder();
    void createFolder();
    void startResumableUpload();
    void putBytes(const QString& sessionUri);
    void applySharing();
    void applyNextRecipient();
    void postPermission(const QJsonObject& permission,
                        std::function<void(bool)> done);
    void fetchLink();
    void finalizeSuccess();
    void fail(const QString& message);

    QNetworkRequest authorizedRequest(const QUrl& url) const;
    QString visibilityDescription() const;

    QNetworkAccessManager* m_net;
    QString m_token;
    QString m_folderId;
    QString m_fileId;
    QString m_fileName;
    QByteArray m_pngData;
    QString m_visibility;
    QStringList m_recipients;
    int m_recipientIndex;
    QString m_sharingWarning;
    bool m_triedFolderRediscovery;
    bool m_waitingForAuth;
    QPushButton* m_cancelButton;
};
