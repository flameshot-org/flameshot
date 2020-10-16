// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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
#include "imgs3settings.h"
#include "src/core/controller.h"
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
#include <QShortcut>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

ImgS3Uploader::ImgS3Uploader(const QPixmap& capture, QWidget* parent)
  : ImgUploader(capture, parent)
{
    init(tr("Upload image to S3"), tr("Uploading Image"));
}

ImgS3Uploader::ImgS3Uploader(QWidget* parent)
  : ImgUploader(parent)
{
    init(tr("Delete image from S3"), tr("Deleting image..."));
}

ImgS3Uploader::~ImgS3Uploader()
{
    clearProxy();
}

void ImgS3Uploader::init(const QString& title, const QString& label)
{
    m_proxy = nullptr;
    m_NetworkAMUpload = nullptr;
    m_NetworkAMGetCreds = nullptr;
    m_NetworkAMRemove = nullptr;

    resultStatus = false;
    setWindowTitle(title);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
}

QNetworkProxy* ImgS3Uploader::proxy()
{
    if (m_proxy == nullptr) {
        initProxy();
    }
    return m_proxy;
}

QNetworkProxy* ImgS3Uploader::initProxy()
{
    // get s3 settings
    ImgS3Settings imgS3Settings;

    // get proxy settings from "config.ini" file
    QSettings* settings = imgS3Settings.settings();
    QString httpProxyHost = settings->value("HTTP_PROXY_HOST").toString();

    if (httpProxyHost.length() > 0) {
        m_proxy = new QNetworkProxy();
        if (settings->contains("HTTP_PROXY_TYPE")) {
            switch (settings->value("HTTP_PROXY_TYPE").toInt()) {
                case 0:
                    m_proxy->setType(QNetworkProxy::DefaultProxy);
                    break;
                case 1:
                    m_proxy->setType(QNetworkProxy::Socks5Proxy);
                    break;
                case 2:
                    m_proxy->setType(QNetworkProxy::NoProxy);
                    break;
                case 4:
                    m_proxy->setType(QNetworkProxy::HttpCachingProxy);
                    break;
                case 5:
                    m_proxy->setType(QNetworkProxy::FtpCachingProxy);
                    break;
                case 3:
                default:
                    m_proxy->setType(QNetworkProxy::HttpProxy);
                    break;
            }
        }

        m_proxy->setHostName(httpProxyHost);
        int nProxyPort = 3128;
        if (settings->contains("HTTP_PROXY_PORT")) {
            nProxyPort = settings->value("HTTP_PROXY_PORT").toInt();
        }
        m_proxy->setPort(nProxyPort);

        if (settings->contains("HTTP_PROXY_USER")) {
            qDebug() << "Proxy user"
                     << settings->value("HTTP_PROXY_PASSWORD").toString();
            m_proxy->setUser(settings->value("HTTP_PROXY_USER").toString());
        }
        if (settings->contains("HTTP_PROXY_PASSWORD")) {
            qDebug() << "Proxy password is not empty";
            m_proxy->setPassword(
              settings->value("HTTP_PROXY_PASSWORD").toString());
        }
    } else {
        // Get proxy settings from OS settings
        QNetworkProxyQuery q(QUrl(m_s3Settings.credsUrl().toUtf8()));
        q.setQueryType(QNetworkProxyQuery::UrlRequest);
        q.setProtocolTag("http");

        QList<QNetworkProxy> proxies =
          QNetworkProxyFactory::systemProxyForQuery(q);
        if (proxies.size() > 0 && proxies[0].type() != QNetworkProxy::NoProxy) {
            m_proxy = new QNetworkProxy();
            m_proxy->setHostName(proxies[0].hostName());
            m_proxy->setPort(proxies[0].port());
            m_proxy->setType(proxies[0].type());
            m_proxy->setUser(proxies[0].user());
            m_proxy->setPassword(proxies[0].password());
        }
    }
#ifdef QT_DEBUG
    if (m_proxy != nullptr) {
        qDebug() << "Using proxy server";
        qDebug() << "proxy host:" << m_proxy->hostName();
        qDebug() << "proxy port:" << m_proxy->port();
        qDebug() << "proxy type:" << m_proxy->type();
        qDebug() << "proxy user:"
                 << (m_proxy->user().length() > 0 ? m_proxy->user()
                                                  : "no user");
        qDebug() << "proxy password:"
                 << (m_proxy->password().length() > 0 ? "***" : "no password");
    } else {
        qDebug() << "No proxy";
    }
#endif
    return m_proxy;
}

void ImgS3Uploader::clearProxy()
{
    if (m_proxy != nullptr) {
        delete m_proxy;
        m_proxy = nullptr;
    }
}

void ImgS3Uploader::handleReplyUpload(QNetworkReply* reply)
{
    hideSpinner();
    m_storageImageName.clear();
    if (reply->error() == QNetworkReply::NoError) {
        // save history
        QString imageName = imageUrl().toString();
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
        resultStatus = true;

        // Copy url to clipboard if required
        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            QApplication::clipboard()->setText(imageUrl().toString());
            SystemNotification().sendMessage(tr("URL copied to clipboard."));
            Controller::getInstance()->updateRecentScreenshots();
            close();
        } else {
            onUploadOk();
        }
    } else {
        QString reason =
          reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute)
            .toString();
        setInfoLabelText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::handleReplyDeleteResource(QNetworkReply* reply)
{
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
        message += "\n\n" + reply->errorString();
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
            setInfoLabelText(reply->errorString());
        }
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::uploadToS3(QJsonDocument& response)
{
    // set paramets from "fields"
    QHttpMultiPart* multiPart =
      new QHttpMultiPart(QHttpMultiPart::FormDataType);

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
        multiPart->append(part);
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
    multiPart->append(imagePart);

    setImageUrl(QUrl(resultUrl));

    QUrl qUrl(url);
    QNetworkRequest request(qUrl);

    // upload
    m_NetworkAMUpload->post(request, multiPart);
}

void ImgS3Uploader::deleteResource(const QString& fileName,
                                   const QString& deleteToken)
{
    // read network settings on each call to simplify configuration management
    // without restarting
    clearProxy();
    if (m_NetworkAMRemove != nullptr) {
        delete m_NetworkAMRemove;
        m_NetworkAMRemove = nullptr;
    }
    m_NetworkAMRemove = new QNetworkAccessManager(this);
    connect(m_NetworkAMRemove,
            &QNetworkAccessManager::finished,
            this,
            &ImgS3Uploader::handleReplyDeleteResource);
    if (proxy() != nullptr) {
        m_NetworkAMRemove->setProxy(*proxy());
    }

    QNetworkRequest request;
    m_storageImageName = fileName;
    m_deleteToken = deleteToken;
    request.setUrl(m_s3Settings.credsUrl().toUtf8() + fileName);
    request.setRawHeader("X-API-Key", m_s3Settings.xApiKey().toLatin1());
    request.setRawHeader("Authorization", "Bearer " + deleteToken.toLatin1());
    m_NetworkAMRemove->deleteResource(request);
}

void ImgS3Uploader::upload()
{
    m_deleteToken.clear();
    m_storageImageName.clear();

    // read network settings on each call to simplify configuration management
    // without restarting init creds and upload network access managers
    clearProxy();
    if (m_NetworkAMGetCreds != nullptr) {
        delete m_NetworkAMGetCreds;
        m_NetworkAMGetCreds = nullptr;
    }
    m_NetworkAMGetCreds = new QNetworkAccessManager(this);
    connect(m_NetworkAMGetCreds,
            &QNetworkAccessManager::finished,
            this,
            &ImgS3Uploader::handleReplyGetCreds);

    if (m_NetworkAMUpload != nullptr) {
        delete m_NetworkAMUpload;
        m_NetworkAMUpload = nullptr;
    }
    m_NetworkAMUpload = new QNetworkAccessManager(this);
    connect(m_NetworkAMUpload,
            &QNetworkAccessManager::finished,
            this,
            &ImgS3Uploader::handleReplyUpload);
    if (proxy() != nullptr) {
        m_NetworkAMGetCreds->setProxy(*proxy());
        m_NetworkAMUpload->setProxy(*proxy());
    }

    // get creads
    QUrl creds(m_s3Settings.credsUrl());
    QNetworkRequest requestCreds(creds);
    if (m_s3Settings.xApiKey().length() > 0) {
        requestCreds.setRawHeader(
          QByteArray("X-API-Key"),
          QByteArray(m_s3Settings.xApiKey().toLocal8Bit()));
    }
    m_NetworkAMGetCreds->get(requestCreds);
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
