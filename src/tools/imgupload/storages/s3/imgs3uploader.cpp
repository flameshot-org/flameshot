// Copyright(c) 2017-2021 Namecheap inc.
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "imgs3uploader.h"
#include "src/core/flameshot.h"
#include "src/utils/confighandler.h"
#include "src/utils/history.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDir>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QThread>
#include <QUrlQuery>
#include <QVBoxLayout>

#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#include <QWidget>
#endif

ImgS3Uploader::ImgS3Uploader(const QPixmap& capture, QWidget* parent)
  : ImgUploaderBase(capture, parent)
{
    init(tr("Upload image to S3"), tr("Uploading Image..."));
}

ImgS3Uploader::ImgS3Uploader(QWidget* parent)
  : ImgUploaderBase(QPixmap(), parent)
{
    init(tr("Delete image from S3"), tr("Deleting image..."));
}

ImgS3Uploader::~ImgS3Uploader()
{
    clearProxy();
    if (nullptr != m_networkAMConfig) {
        delete m_networkAMConfig;
    }
    if (nullptr != m_networkAMUpload) {
        m_networkAMUpload->disconnect();
        delete m_networkAMUpload;
    }
    if (nullptr != m_networkAMGetCreds) {
        m_networkAMGetCreds->disconnect();
        delete m_networkAMGetCreds;
    }
    if (nullptr != m_networkAMRemove) {
        m_networkAMRemove->disconnect();
        delete m_networkAMRemove;
    }
    if (nullptr != m_multiPart) {
        delete m_multiPart;
    }
}

void ImgS3Uploader::init(const QString& title, const QString& label)
{
    m_multiPart = nullptr;
    m_networkAMUpload = nullptr;
    m_networkAMGetCreds = nullptr;
    m_networkAMRemove = nullptr;
    m_networkAMConfig = nullptr;

    resultStatus = false;
    setWindowTitle(title);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
}

QNetworkProxy* ImgS3Uploader::proxy()
{
    return m_s3Settings.proxy();
}

void ImgS3Uploader::clearProxy()
{
    m_s3Settings.clearProxy();
}

void ImgS3Uploader::handleReplyPostUpload(QNetworkReply* reply)
{
    spinner()->hide();
    m_storageImageName.clear();
    if (reply->error() == QNetworkReply::NoError) {
        // save history
        QString imageName = imageURL().toString();
        int lastSlash = imageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            imageName = imageName.mid(lastSlash + 1);
        }
        m_storageImageName = imageName;

        // save image to history
        History history;
        imageName = history.packFileName(
          SCREENSHOT_STORAGE_TYPE_S3, m_deleteToken, imageName);
        history.save(pixmap(), imageName);
        m_currentImageName = imageName;
        resultStatus = true;

        // Copy url to clipboard if required
        if (ConfigHandler().copyAndCloseAfterUpload()) {
            SystemNotification().sendMessage(tr("URL copied to clipboard."));

            // FIXME - bring it back if required
            //            Controller::getInstance()->showRecentUploads();
            //            Controller::getInstance()->updateRecentScreenshots();

            QApplication::clipboard()->setText(imageURL().toString());
            close();
        }
        emit uploadOk(imageURL());
    } else {
        onUploadError(reply);
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::onUploadError(QNetworkReply* reply)
{
    Q_UNUSED(reply)
    spinner()->hide();
    if (QMessageBox::Retry ==
        QMessageBox::question(nullptr,
                              tr("Error"),
                              tr("Unable to upload screenshot, please check "
                                 "your internet connection and try again"),
                              QMessageBox::Retry | QMessageBox::Cancel)) {
        upload();
        return;
    }
    hide();
}

void ImgS3Uploader::handleReplyDeleteResource(QNetworkReply* reply)
{
    // Just remove image preview on fail because it can be outdated and removed
    // on server.
    removeImagePreview();
    close();

    /* The following code possibly will be required later so it is just
     * commented here */
    auto replyError = reply->error();
    if (replyError == QNetworkReply::NoError) {
        removeImagePreview();
    } else {
        hide();

        // generate error message
        QString message =
          tr("Unable to remove screenshot from the remote storage.");
        if (replyError == QNetworkReply::UnknownNetworkError) {
            message += "\n" + tr("Network error");
        } else if (replyError == QNetworkReply::UnknownServerError) {
            message += "\n" + tr("Possibly it doesn't exist anymore");
        }
        message +=
          "\n\n" +
          tr("Do you want to remove screenshot from local history anyway?");

        if (QMessageBox::Yes ==
            QMessageBox::question(NULL,
                                  tr("Remove screenshot from history?"),
                                  message,
                                  QMessageBox::Yes | QMessageBox::No)) {
            removeImagePreview();
        }
    }
    close();
}

void ImgS3Uploader::handleReplyGetCreds(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        uploadToS3(response);
    } else {
        if (m_s3Settings.credsUrl().length() == 0) {
            setInfoLabelText(
              tr("S3 Creds URL is not found in your configuration file"));
        } else {
            onUploadError(reply);
            return;
        }
        // FIXME - remove not uploaded preview
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::uploadToS3(QJsonDocument& response)
{
    if (nullptr == m_networkAMUpload) {
        m_networkAMUpload = new QNetworkAccessManager(this);
        connect(m_networkAMUpload,
                &QNetworkAccessManager::finished,
                this,
                &ImgS3Uploader::handleReplyPostUpload);
    }
    if (proxy() != nullptr) {
        m_networkAMUpload->setProxy(*proxy());
    } else {
        m_networkAMUpload->setProxy(QNetworkProxy());
    }

    // set parameters from "fields"
    if (nullptr != m_multiPart) {
        delete m_multiPart;
        m_multiPart = nullptr;
    }
    m_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // read JSON response
    QJsonObject json = response.object();
    QString resultUrl = json["resultURL"].toString();
    QJsonObject formData = json["formData"].toObject();
    QString url = formData["url"].toString();
    m_deleteToken = json["deleteToken"].toString();

    QJsonObject fields = formData["fields"].toObject();
    foreach (auto key, fields.keys()) {
        QString field = fields[key].toString();
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"" + key + "\""));
        part.setBody(field.toLatin1());
        m_multiPart->append(part);
    }

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("image/png"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"file\""));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap().save(&buffer, "PNG");

    imagePart.setBody(byteArray);
    m_multiPart->append(imagePart);

    setImageURL(QUrl(resultUrl));

    QUrl qUrl(url);
    QNetworkRequest request(qUrl);

    // upload
    m_networkAMUpload->post(request, m_multiPart);
}

void ImgS3Uploader::deleteImage(const QString& fileName,
                                const QString& deleteToken)
{
    // read network settings on each call to simplify configuration management
    // without restarting
    clearProxy();
    if (m_networkAMRemove == nullptr) {
        m_networkAMRemove = new QNetworkAccessManager(this);
        connect(m_networkAMRemove,
                &QNetworkAccessManager::finished,
                this,
                &ImgS3Uploader::handleReplyDeleteResource);
    }
    if (proxy() != nullptr) {
        m_networkAMRemove->setProxy(*proxy());
    } else {
        m_networkAMRemove->setProxy(QNetworkProxy());
    }

    QNetworkRequest request;
    m_storageImageName = fileName;
    m_deleteToken = deleteToken;

    request.setUrl(m_s3Settings.credsUrl().toUtf8() + fileName);
    request.setRawHeader("X-API-Key", m_s3Settings.xApiKey().toLatin1());
    request.setRawHeader("Authorization", "Bearer " + deleteToken.toLatin1());
    m_networkAMRemove->deleteResource(request);

    emit deleteOk();
}

void ImgS3Uploader::upload()
{
    m_deleteToken.clear();
    m_storageImageName.clear();

    // read network settings on each call to simplify configuration management
    // without restarting init creds and upload network access managers
    clearProxy();

    // check for outdated s3 creds
    QString credsUpdated =
      m_s3Settings.value("S3", "S3_CREDS_UPDATED").toString();
    QDateTime dtCredsUpdated =
      QDateTime::currentDateTime().fromString(credsUpdated, Qt::ISODate);
    QDateTime now = QDateTime::currentDateTime();
    dtCredsUpdated = dtCredsUpdated.addDays(1);
    if (m_s3Settings.credsUrl().isEmpty() || dtCredsUpdated <= now) {
        getConfigRemote();
        if (m_s3Settings.credsUrl().isEmpty()) {
            // no creds, even outdated, need to get creds first, cannot continue
            return;
        }
    }

    // clean old network connections and start uploading
    if (nullptr == m_networkAMGetCreds) {
        m_networkAMGetCreds = new QNetworkAccessManager(this);
        connect(m_networkAMGetCreds,
                &QNetworkAccessManager::finished,
                this,
                &ImgS3Uploader::handleReplyGetCreds);
    }
    if (proxy() != nullptr) {
        m_networkAMGetCreds->setProxy(*proxy());
    } else {
        m_networkAMGetCreds->setProxy(QNetworkProxy());
    }

    // get creds
    QNetworkRequest requestCreds(QUrl(m_s3Settings.credsUrl()));
    if (m_s3Settings.xApiKey().length() > 0) {
        requestCreds.setRawHeader(
          QByteArray("X-API-Key"),
          QByteArray(m_s3Settings.xApiKey().toLocal8Bit()));
    }
    m_networkAMGetCreds->get(requestCreds);
#if defined(Q_OS_MACOS)
    // Hide capture widget on MacOS
    for (QWidget* widget : qApp->topLevelWidgets()) {
        QString className(widget->metaObject()->className());
        if (0 ==
            className.compare(CaptureWidget::staticMetaObject.className())) {
            widget->showNormal();
            widget->hide();
            break;
        }
    }
#endif
}

void ImgS3Uploader::removeImagePreview()
{
    // remove local file
    History history;
    QString packedFileName = history.packFileName(
      SCREENSHOT_STORAGE_TYPE_S3, m_deleteToken, m_storageImageName);
    QString fullFileName = history.path() + packedFileName;

    QFile file(fullFileName);
    if (file.exists()) {
        file.remove();
    }
    m_deleteToken.clear();
    m_storageImageName.clear();
    resultStatus = true;
}

void ImgS3Uploader::getConfigRemote()
{
    if (nullptr == m_networkAMConfig) {
        m_networkAMConfig = new QNetworkAccessManager(this);
        connect(m_networkAMConfig,
                &QNetworkAccessManager::finished,
                this,
                &ImgS3Uploader::handleReplyGetConfig);
    }
    if (proxy() != nullptr) {
        m_networkAMConfig->setProxy(*proxy());
    } else {
        m_networkAMConfig->setProxy(QNetworkProxy());
    }
    QNetworkRequest requestConfig(m_s3Settings.configUrl());
    m_networkAMConfig->get(requestConfig);
}

void ImgS3Uploader::handleReplyGetConfig(QNetworkReply* reply)
{
    if (nullptr == m_networkAMConfig) {
        delete m_networkAMConfig;
        m_networkAMConfig = nullptr;
    }
    if (reply->error() == QNetworkReply::NoError) {
        bool doUpload = m_s3Settings.credsUrl().isEmpty();
        QString data = QString(reply->readAll());
        m_s3Settings.updateConfigurationData(data);
        if (doUpload) {
            upload();
        } else {
            hide();
        }
    } else if (m_s3Settings.credsUrl().isEmpty()) {
#if defined(Q_OS_MACOS)
        // Hide capture widget on MacOS (exit from full-screen mode & hide)
        for (QWidget* widget : qApp->topLevelWidgets()) {
            QString className(widget->metaObject()->className());
            if (0 == className.compare(
                       CaptureWidget::staticMetaObject.className())) {
                widget->showNormal();
                widget->hide();
                break;
            }
        }
#endif

        QString message = reply->errorString() + "\n\n" +
                          tr("Unable to get s3 credentials, please check "
                             "your VPN connection and try again");
        if (QMessageBox::Retry ==
            QMessageBox::question(nullptr,
                                  tr("Error"),
                                  message,
                                  QMessageBox::Retry | QMessageBox::Cancel)) {
            setInfoLabelText(
              tr("Retrieving configuration file with s3 creds..."));
            getConfigRemote();
            return;
        }
        hide();
    }
}
