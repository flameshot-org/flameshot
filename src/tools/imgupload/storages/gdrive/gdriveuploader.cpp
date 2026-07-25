// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#include "gdriveuploader.h"
#include "gdriveoauth.h"
#include "utils/confighandler.h"
#include "utils/filenamehandler.h"
#include "utils/history.h"
#include "widgets/loadspinner.h"
#include "widgets/notificationwidget.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QShortcut>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <functional>

namespace {
const char* kFilesEndpoint = "https://www.googleapis.com/drive/v3/files";
const char* kUploadEndpoint =
  "https://www.googleapis.com/upload/drive/v3/files";
const char* kFolderMime = "application/vnd.google-apps.folder";
// A constant name identical for every user (R5).
const char* kFolderName = "Flameshot screenshots";
constexpr int kRequestTimeoutMs = 60 * 1000;

QJsonObject replyJson(QNetworkReply* reply)
{
    return QJsonDocument::fromJson(reply->readAll()).object();
}
}

GDriveUploader::GDriveUploader(const QPixmap& capture, QWidget* parent)
  : ImgUploaderBase(capture, parent)
  , m_net(new QNetworkAccessManager(this))
  , m_recipientIndex(0)
  , m_triedFolderRediscovery(false)
  , m_waitingForAuth(false)
  , m_cancelButton(nullptr)
{
    m_visibility = ConfigHandler().gdriveDefaultVisibility();
    m_folderId = ConfigHandler().gdriveFolderId();
}

GDriveUploader::~GDriveUploader()
{
    // Closing the widget mid-consent detaches this waiter; the shared listener
    // stops once no waiters remain (KTD9).
    if (m_waitingForAuth) {
        GDriveOAuth::instance()->detachWaiter();
    }
}

void GDriveUploader::setVisibility(const QString& visibility)
{
    m_visibility = visibility;
}

void GDriveUploader::setRecipients(const QStringList& recipients)
{
    m_recipients = recipients;
}

QNetworkRequest GDriveUploader::authorizedRequest(const QUrl& url) const
{
    QNetworkRequest request{ url };
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(m_token).toUtf8());
    request.setTransferTimeout(kRequestTimeoutMs);
    return request;
}

void GDriveUploader::withAccessToken(
  std::function<void(const QString&)> onReady,
  std::function<void()> onCanceled,
  std::function<void(const QString&)> onFailed)
{
    GDriveOAuth* oauth = GDriveOAuth::instance();
    m_waitingForAuth = true;
    oauth->attachWaiter();

    connect(oauth,
            &GDriveOAuth::accessTokenReady,
            this,
            [this, oauth, onReady](const QString& token) {
                disconnect(oauth, nullptr, this, nullptr);
                m_waitingForAuth = false;
                oauth->detachWaiter();
                onReady(token);
            });
    connect(
      oauth, &GDriveOAuth::authCanceled, this, [this, oauth, onCanceled]() {
          disconnect(oauth, nullptr, this, nullptr);
          m_waitingForAuth = false;
          oauth->detachWaiter();
          onCanceled();
      });
    connect(oauth,
            &GDriveOAuth::authFailed,
            this,
            [this, oauth, onFailed](const QString& error) {
                disconnect(oauth, nullptr, this, nullptr);
                m_waitingForAuth = false;
                oauth->detachWaiter();
                onFailed(error);
            });

    oauth->requestAccessToken();
}

void GDriveUploader::upload()
{
    // Prepare the PNG bytes and target filename up front.
    QBuffer buffer(&m_pngData);
    buffer.open(QIODevice::WriteOnly);
    pixmap().save(&buffer, "PNG");
    m_fileName = FileNameHandler().parsedPattern() + QStringLiteral(".png");

    // Show a cancel affordance while we wait for authorization (R15).
    setInfoLabelText(tr("Waiting for Google authorization…"));
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    if (auto* boxLayout = qobject_cast<QVBoxLayout*>(layout())) {
        boxLayout->addWidget(m_cancelButton);
    }
    connect(m_cancelButton, &QPushButton::clicked, this, &QWidget::close);

    withAccessToken(
      [this](const QString& token) {
          if (m_cancelButton) {
              m_cancelButton->hide();
          }
          setInfoLabelText(tr("Uploading image…"));
          m_token = token;
          ensureFolder();
      },
      [this]() {
          // User denial / cancel renders as a plain cancellation (KTD9).
          if (spinner()) {
              spinner()->deleteLater();
          }
          if (m_cancelButton) {
              m_cancelButton->hide();
          }
          setInfoLabelText(tr("Upload canceled."));
          new QShortcut(Qt::Key_Escape, this, SLOT(close()));
      },
      [this](const QString& error) { fail(error); });
}

void GDriveUploader::ensureFolder()
{
    if (!m_folderId.isEmpty()) {
        // Trust the cached ID; a stale ID surfaces as a 404 at upload time and
        // triggers re-discovery (KTD5).
        startResumableUpload();
    } else {
        findOrCreateFolder();
    }
}

void GDriveUploader::findOrCreateFolder()
{
    // Under drive.file the app only ever sees folders it created, so listing is
    // scoped to our own folder (R7). First run finds nothing and creates it.
    const QString filter =
      QStringLiteral("name = '%1' and mimeType = '%2' and trashed = false")
        .arg(QString::fromLatin1(kFolderName),
             QString::fromLatin1(kFolderMime));
    QUrl url{ QString::fromLatin1(kFilesEndpoint) };
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), filter);
    query.addQueryItem(QStringLiteral("fields"),
                       QStringLiteral("files(id,name)"));
    query.addQueryItem(QStringLiteral("spaces"), QStringLiteral("drive"));
    url.setQuery(query);

    QNetworkReply* reply = m_net->get(authorizedRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            fail(tr("Could not access Google Drive."));
            return;
        }
        const QJsonArray files =
          replyJson(reply).value(QStringLiteral("files")).toArray();
        if (!files.isEmpty()) {
            m_folderId =
              files.first().toObject().value(QStringLiteral("id")).toString();
            ConfigHandler().setGdriveFolderId(m_folderId);
            startResumableUpload();
        } else {
            createFolder();
        }
    });
}

void GDriveUploader::createFolder()
{
    QJsonObject metadata;
    metadata[QStringLiteral("name")] = QString::fromLatin1(kFolderName);
    metadata[QStringLiteral("mimeType")] = QString::fromLatin1(kFolderMime);

    QNetworkRequest request =
      authorizedRequest(QUrl{ QString::fromLatin1(kFilesEndpoint) });
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json; charset=UTF-8"));

    QNetworkReply* reply = m_net->post(
      request, QJsonDocument(metadata).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QString id =
          replyJson(reply).value(QStringLiteral("id")).toString();
        if (reply->error() != QNetworkReply::NoError || id.isEmpty()) {
            fail(tr("Could not create the \"%1\" folder on Google Drive.")
                   .arg(QString::fromLatin1(kFolderName)));
            return;
        }
        m_folderId = id;
        ConfigHandler().setGdriveFolderId(m_folderId);
        startResumableUpload();
    });
}

void GDriveUploader::startResumableUpload()
{
    // Resumable protocol (KTD4): initiate with a metadata POST, then PUT bytes
    // to the returned session URI. Avoids the 5 MB multipart cap.
    QJsonObject metadata;
    metadata[QStringLiteral("name")] = m_fileName;
    QJsonArray parents;
    parents.append(m_folderId);
    metadata[QStringLiteral("parents")] = parents;

    QUrl url{ QString::fromLatin1(kUploadEndpoint) };
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("uploadType"),
                       QStringLiteral("resumable"));
    url.setQuery(query);

    QNetworkRequest request = authorizedRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json; charset=UTF-8"));
    request.setRawHeader("X-Upload-Content-Type", "image/png");

    QNetworkReply* reply = m_net->post(
      request, QJsonDocument(metadata).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const int status =
          reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            // A stale cached folder ID shows up as 404 here: drop it and
            // rediscover once (KTD5).
            if (status == 404 && !m_triedFolderRediscovery) {
                m_triedFolderRediscovery = true;
                m_folderId.clear();
                ConfigHandler().setGdriveFolderId(QStringLiteral(""));
                findOrCreateFolder();
                return;
            }
            if (status == 403) {
                fail(tr("Google Drive refused the upload (quota full or "
                        "blocked by policy)."));
                return;
            }
            fail(tr("The upload to Google Drive failed."));
            return;
        }
        const QByteArray location = reply->rawHeader("Location");
        if (location.isEmpty()) {
            fail(tr("Google Drive did not return an upload session."));
            return;
        }
        putBytes(QString::fromUtf8(location));
    });
}

void GDriveUploader::putBytes(const QString& sessionUri)
{
    QNetworkRequest request{ QUrl(sessionUri) };
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("image/png"));
    request.setTransferTimeout(kRequestTimeoutMs);

    QNetworkReply* reply = m_net->put(request, m_pngData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_fileId = replyJson(reply).value(QStringLiteral("id")).toString();
        if (reply->error() != QNetworkReply::NoError || m_fileId.isEmpty()) {
            fail(tr("The upload to Google Drive did not complete."));
            return;
        }
        applySharing();
    });
}

void GDriveUploader::applySharing()
{
    // Private: no permission call; the file stays owner-only.
    if (m_visibility == QStringLiteral("private")) {
        fetchLink();
        return;
    }

    if (m_visibility == QStringLiteral("anyone")) {
        QJsonObject permission{
            { QStringLiteral("type"), QStringLiteral("anyone") },
            { QStringLiteral("role"), QStringLiteral("reader") }
        };
        postPermission(permission, [this](bool ok) {
            if (!ok) {
                m_sharingWarning =
                  tr("The file was uploaded but could not be made public "
                     "(blocked by your organization's policy?). It stays "
                     "private; the link still works for you.");
            }
            fetchLink();
        });
        return;
    }

    if (m_visibility == QStringLiteral("domain")) {
        GDriveOAuth* oauth = GDriveOAuth::instance();
        const QString domain = oauth->accountDomain();
        if (domain.isEmpty()) {
            // A known account with no domain is a personal Google account: it
            // has no organization, which is a different situation from never
            // having learned the domain (sign-in permission declined).
            m_sharingWarning =
              oauth->accountEmail().isEmpty()
                ? tr("Flameshot could not read your organization's domain "
                     "because the sign-in permission was declined, so the file "
                     "stays private. Reconnect the account in the settings to "
                     "enable organization sharing. The link still works for "
                     "you.")
                : tr("This Google account is not part of an organization, so "
                     "there is nobody to share with and the file stays "
                     "private. The link still works for you.");
            fetchLink();
            return;
        }
        QJsonObject permission{
            { QStringLiteral("type"), QStringLiteral("domain") },
            { QStringLiteral("role"), QStringLiteral("reader") },
            { QStringLiteral("domain"), domain }
        };
        postPermission(permission, [this](bool ok) {
            if (!ok) {
                m_sharingWarning =
                  tr("The file was uploaded but organization sharing could "
                     "not be applied (blocked by policy?). It stays private; "
                     "the link still works for you.");
            }
            fetchLink();
        });
        return;
    }

    if (m_visibility == QStringLiteral("users")) {
        if (m_recipients.isEmpty()) {
            // e.g. a "users" default with upload-without-confirmation and no
            // per-upload recipients: don't claim success with nobody shared.
            m_sharingWarning =
              tr("No recipients were specified, so the file stays private. "
                 "The link still works for you.");
            fetchLink();
            return;
        }
        m_recipientIndex = 0;
        applyNextRecipient();
        return;
    }

    // Unknown level: behave like private.
    fetchLink();
}

void GDriveUploader::applyNextRecipient()
{
    if (m_recipientIndex >= m_recipients.size()) {
        fetchLink();
        return;
    }
    const QString email = m_recipients.at(m_recipientIndex).trimmed();
    if (email.isEmpty()) {
        ++m_recipientIndex;
        applyNextRecipient();
        return;
    }

    QJsonObject asUser{ { QStringLiteral("type"), QStringLiteral("user") },
                        { QStringLiteral("role"), QStringLiteral("reader") },
                        { QStringLiteral("emailAddress"), email } };
    postPermission(asUser, [this, email](bool ok) {
        if (ok) {
            ++m_recipientIndex;
            applyNextRecipient();
            return;
        }
        // The typed address may be a group rather than a user: retry as group.
        QJsonObject asGroup{
            { QStringLiteral("type"), QStringLiteral("group") },
            { QStringLiteral("role"), QStringLiteral("reader") },
            { QStringLiteral("emailAddress"), email }
        };
        postPermission(asGroup, [this, email](bool ok2) {
            if (!ok2) {
                if (!m_sharingWarning.isEmpty()) {
                    m_sharingWarning += QStringLiteral("\n");
                }
                m_sharingWarning += tr("Could not share with %1.").arg(email);
            }
            ++m_recipientIndex;
            applyNextRecipient();
        });
    });
}

void GDriveUploader::postPermission(const QJsonObject& permission,
                                    std::function<void(bool)> done)
{
    QUrl url{ QStringLiteral("%1/%2/permissions")
                .arg(QString::fromLatin1(kFilesEndpoint), m_fileId) };
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("sendNotificationEmail"),
                       QStringLiteral("false"));
    url.setQuery(query);
    QNetworkRequest request = authorizedRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json; charset=UTF-8"));
    QNetworkReply* reply = m_net->post(
      request, QJsonDocument(permission).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [reply, done]() {
        reply->deleteLater();
        done(reply->error() == QNetworkReply::NoError);
    });
}

void GDriveUploader::fetchLink()
{
    QUrl url{ QStringLiteral("%1/%2").arg(QString::fromLatin1(kFilesEndpoint),
                                          m_fileId) };
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("fields"), QStringLiteral("webViewLink"));
    url.setQuery(query);

    QNetworkReply* reply = m_net->get(authorizedRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        QString link =
          replyJson(reply).value(QStringLiteral("webViewLink")).toString();
        if (link.isEmpty()) {
            link = History::driveFileUrl(m_fileId);
        }
        setImageURL(QUrl(link));
        finalizeSuccess();
    });
}

void GDriveUploader::finalizeSuccess()
{
    if (spinner()) {
        spinner()->deleteLater();
    }

    // Pack history with the Drive file ID hex-encoded in the token slot, since
    // raw IDs contain '-' (KTD8).
    History history;
    const QString token = History::encodeDriveFileId(m_fileId);
    m_currentImageName =
      history.packFileName(QStringLiteral("gdrive"), token, m_fileName);
    history.save(pixmap(), m_currentImageName);

    emit uploadOk(imageURL());

    // Name the applied visibility (and any sharing warning) in the result,
    // including on the no-confirmation path. The post-upload dialog owns a
    // notification widget only when the copy-URL setting shows it; otherwise
    // the inline info label is the fallback (KTD7, R14).
    QString message = m_sharingWarning.isEmpty()
                        ? tr("Shared: %1").arg(visibilityDescription())
                        : m_sharingWarning;
    if (ConfigHandler().copyURLAfterUpload() && notification()) {
        notification()->showMessage(message);
    } else {
        setInfoLabelText(message);
    }
}

void GDriveUploader::fail(const QString& message)
{
    if (spinner()) {
        spinner()->deleteLater();
    }
    if (m_cancelButton) {
        m_cancelButton->hide();
    }
    setInfoLabelText(message);
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

QString GDriveUploader::visibilityDescription() const
{
    if (m_visibility == QStringLiteral("private")) {
        return tr("private (only you)");
    }
    if (m_visibility == QStringLiteral("anyone")) {
        return tr("anyone on the internet with the link");
    }
    if (m_visibility == QStringLiteral("users")) {
        return tr("specific people");
    }
    return tr("anyone in your organization with the link");
}

void GDriveUploader::deleteImage(const QString& fileName,
                                 const QString& deleteToken)
{
    Q_UNUSED(fileName)
    // The Drive file ID is stored hex-encoded in the token slot (KTD8).
    const QString fileId = History::decodeDriveFileId(deleteToken);
    if (fileId.isEmpty()) {
        emit deleteFail(tr("This history entry has no Google Drive file ID."));
        return;
    }

    // May run headless from a history row (no visible widget): a delete needing
    // re-auth triggers the same shared consent flow (KTD9).
    withAccessToken(
      [this, fileId](const QString& token) {
          m_token = token;
          QUrl url{ QStringLiteral("%1/%2").arg(
            QString::fromLatin1(kFilesEndpoint), fileId) };
          QNetworkReply* reply = m_net->deleteResource(authorizedRequest(url));
          connect(reply, &QNetworkReply::finished, this, [this, reply]() {
              reply->deleteLater();
              const int status =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                  .toInt();
              // 204 No Content on success; treat an already-gone file (404) as
              // deleted too.
              if (reply->error() == QNetworkReply::NoError || status == 404) {
                  if (isVisible() && notification()) {
                      notification()->showMessage(
                        tr("Screenshot deleted from Google Drive."));
                  }
                  emit deleteOk();
              } else {
                  // No message from here: the shared post-upload dialog reports
                  // every delete failure on deleteFail, so a message here would
                  // double up on this path while leaving the authorization
                  // paths below silent (KTD4).
                  emit deleteFail(
                    tr("Could not delete the file from Google Drive."));
              }
          });
      },
      [this]() {
          emit deleteFail(
            tr("Authorization was canceled; the file was not deleted."));
      },
      [this](const QString& error) { emit deleteFail(error); });
}
