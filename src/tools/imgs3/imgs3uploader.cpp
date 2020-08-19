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
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/notificationwidget.h"
#include "src/utils/confighandler.h"
#include "src/utils/history.h"
#include "src/utils/configenterprise.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QShortcut>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QBuffer>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpMultiPart>
#include <QNetworkProxy>
#include <QDir>


ImgS3Uploader::ImgS3Uploader(const QPixmap &capture, QWidget *parent) :
    QWidget(parent), m_pixmap(capture)
{
    init(tr("Upload image to S3"), tr("Uploading Image"));
}

ImgS3Uploader::ImgS3Uploader(QWidget *parent) :
    QWidget(parent)
{
    init(tr("Delete image from S3"), tr("Deleting image..."));
}

void ImgS3Uploader::init(const QString &title, const QString &label) {
    m_imageLabel = nullptr;
    m_spinner = nullptr;

    m_proxy = nullptr;
    m_NetworkAMUpload = nullptr;
    m_NetworkAMGetCreds = nullptr;
    m_NetworkAMRemove = nullptr;

    m_success = false;
    setWindowTitle(title);
    setWindowIcon(QIcon(":img/app/flameshot.svg"));

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiMainColorValue());
    m_spinner->start();

    m_infoLabel = new QLabel(label);
    m_infoLabel->setAlignment(Qt::AlignCenter);

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    setAttribute(Qt::WA_DeleteOnClose);
}

QNetworkProxy *ImgS3Uploader::proxy() {
    if(m_proxy == nullptr) {
        initProxy();
    }
    return m_proxy;
}

QNetworkProxy *ImgS3Uploader::initProxy() {
    // get enterprise settings
    ConfigEnterprise *configEnterprise = new ConfigEnterprise();

    // get proxy settings from "config.ini" file
    QSettings *settings = configEnterprise->settings();
    QString httpProxyHost = settings->value("HTTP_PROXY_HOST").toString();

    if(httpProxyHost.length() > 0) {
        m_proxy = new QNetworkProxy();

        if(settings->contains("HTTP_PROXY_TYPE")) {
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
        if(settings->contains("HTTP_PROXY_PORT")) {
            nProxyPort = settings->value("HTTP_PROXY_PORT").toInt();
        }
        m_proxy->setPort(nProxyPort);

        if(settings->contains("HTTP_PROXY_USER")) {
            qDebug() << "Proxy user" << settings->value("HTTP_PROXY_PASSWORD").toString();
            m_proxy->setUser(settings->value("HTTP_PROXY_USER").toString());
        }
        if(settings->contains("HTTP_PROXY_PASSWORD")) {
            qDebug() << "Proxy password is not empty";
            m_proxy->setPassword(settings->value("HTTP_PROXY_PASSWORD").toString());
        }
    }
    else {
        // Get proxy settings from OS settings
        QNetworkProxyQuery q(QUrl(m_s3Settings.credsUrl().toUtf8()));
        q.setQueryType(QNetworkProxyQuery::UrlRequest);
        q.setProtocolTag("http");

        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(q);
        if( proxies.size() > 0 && proxies[0].type() != QNetworkProxy::NoProxy ){
            m_proxy = new QNetworkProxy();
            m_proxy->setHostName(proxies[0].hostName());
            m_proxy->setPort(proxies[0].port());
            m_proxy->setType(proxies[0].type());
            m_proxy->setUser(proxies[0].user());
            m_proxy->setPassword(proxies[0].password());
        }
    }
#ifdef QT_DEBUG
    if(m_proxy != nullptr) {
        qDebug() << "Using proxy server";
        qDebug() << "proxy host:" << m_proxy->hostName();
        qDebug() << "proxy port:" << m_proxy->port();
        qDebug() << "proxy type:" << m_proxy->type();
        qDebug() << "proxy user:" << (m_proxy->user().length() > 0 ? m_proxy->user() : "no user");
        qDebug() << "proxy password:" << (m_proxy->password().length() > 0 ? "***" : "no password");
    }
    else {
        qDebug() << "No proxy";
    }
#endif
    return m_proxy;
}

void ImgS3Uploader::clearProxy() {
    if(m_proxy != nullptr) {
        delete m_proxy;
        m_proxy = nullptr;
    }
}


void ImgS3Uploader::handleReplyUpload(QNetworkReply *reply) {
    hideSpinner();
    m_s3ImageName.clear();
    if (reply->error() == QNetworkReply::NoError) {
        // save history
        QString imageName = m_imageURL.toString();
        int lastSlash = imageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            imageName = imageName.mid(lastSlash + 1);
        }
        m_s3ImageName = imageName;
        History history;
        imageName = history.packFileName(SCREENSHOT_STORAGE_TYPE_S3, m_deleteToken, imageName);
        history.save(m_pixmap, imageName);
        m_success = true;

        // Copy url to clipboard if required
        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            QApplication::clipboard()->setText(m_imageURL.toString());
            SystemNotification().sendMessage(tr("URL copied to clipboard."));
            close();
        } else {
            onUploadOk();
        }
    } else {
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        setInfoLabelText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::handleReplyDeleteResource(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        m_success = true;

        // remove local file
        History history;
        QString packedFileName = history.packFileName(SCREENSHOT_STORAGE_TYPE_S3, m_deleteToken, m_s3ImageName);
        QString fullFileName = history.path() + packedFileName;

        QFile file(fullFileName);
        if (file.exists()) {
            file.remove();
        }
        m_deleteToken.clear();
        m_s3ImageName.clear();
        close();
    } else {
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        setInfoLabelText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::startDrag() {
    QMimeData *mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl> { m_imageURL });
    mimeData->setImageData(m_pixmap);

    QDrag *dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(256, 256, Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation));
    dragHandler->exec();
}

void ImgS3Uploader::handleReplyGetCreds(QNetworkReply *reply){
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        uploadToS3(response);
    } else {
        if(m_s3Settings.credsUrl().length() == 0){
            setInfoLabelText(tr("S3 Creds URL is not found in your configuration file"));
        }
        else {
            setInfoLabelText(reply->errorString());
        }
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgS3Uploader::uploadToS3(QJsonDocument &response) {
    // set paramets from "fields"
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // read JSON response
    QJsonObject json = response.object();
    QString resultURL = json["resultURL"].toString();
    QJsonObject formData = json["formData"].toObject();
    QString url = formData["url"].toString();
    m_deleteToken = json["deleteToken"].toString();

    QJsonObject fields = formData["fields"].toObject();
    foreach (auto key, fields.keys()) {
        QString field = fields[key].toString();
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + key + "\""));
        part.setBody(field.toLatin1());
        multiPart->append(part);
    }

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"file\""));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    m_pixmap.save(&buffer, "PNG");

    imagePart.setBody(byteArray);
    multiPart->append(imagePart);

    m_imageURL.setUrl(resultURL);

    QUrl qUrl(url);
    QNetworkRequest request(qUrl);

    // upload
    m_NetworkAMUpload->post(request, multiPart);
}

void ImgS3Uploader::deleteResource(const QString &fileName, const QString &deleteToken) {
    // read network settings on each call to simplify configuration management without restarting
    clearProxy();
    if(m_NetworkAMRemove != nullptr) {
        delete m_NetworkAMRemove;
        m_NetworkAMRemove = nullptr;
    }
    m_NetworkAMRemove = new QNetworkAccessManager(this);
    connect(m_NetworkAMRemove, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleReplyDeleteResource);
    if(proxy() != nullptr) {
        m_NetworkAMRemove->setProxy(*proxy());
    }

    QNetworkRequest request;
    m_s3ImageName = fileName;
    m_deleteToken = deleteToken;
    request.setUrl(m_s3Settings.credsUrl().toUtf8() + fileName);
    request.setRawHeader("X-API-Key", m_s3Settings.xApiKey().toLatin1());
    request.setRawHeader("Authorization", "Bearer " + deleteToken.toLatin1());
    m_NetworkAMRemove->deleteResource(request);
}

void ImgS3Uploader::upload() {
    m_deleteToken.clear();
    m_s3ImageName.clear();

    // read network settings on each call to simplify configuration management without restarting
    // init creds and upload network access managers
    clearProxy();
    if(m_NetworkAMGetCreds != nullptr) {
        delete m_NetworkAMGetCreds;
        m_NetworkAMGetCreds = nullptr;
    }
    m_NetworkAMGetCreds = new QNetworkAccessManager(this);
    connect(m_NetworkAMGetCreds, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleReplyGetCreds);

    if(m_NetworkAMUpload != nullptr) {
        delete m_NetworkAMUpload;
        m_NetworkAMUpload = nullptr;
    }
    m_NetworkAMUpload = new QNetworkAccessManager(this);
    connect(m_NetworkAMUpload, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleReplyUpload);
    if(proxy() != nullptr) {
        m_NetworkAMGetCreds->setProxy(*proxy());
        m_NetworkAMUpload->setProxy(*proxy());
    }

    // get creads
    QUrl creds(m_s3Settings.credsUrl());
    QNetworkRequest requestCreds(creds);
    if(m_s3Settings.xApiKey().length() > 0) {
        requestCreds.setRawHeader(QByteArray("X-API-Key"), QByteArray(m_s3Settings.xApiKey().toLocal8Bit()));
    }
    m_NetworkAMGetCreds->get(requestCreds);
}

void ImgS3Uploader::onUploadOk() {
    hideSpinner();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    if(nullptr == m_imageLabel) {
        m_imageLabel = new ImageLabel();
        m_imageLabel->setScreenshot(m_pixmap);
        m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(m_imageLabel, &ImageLabel::dragInitiated, this, &ImgS3Uploader::startDrag);
        m_vLayout->addWidget(m_imageLabel);
    }

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_deleteImageOnS3 = new QPushButton(tr("Delete image"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_openUrlButton);
    m_hLayout->addWidget(m_deleteImageOnS3);
    m_hLayout->addWidget(m_toClipboardButton);

    connect(m_copyUrlButton, &QPushButton::clicked,
            this, &ImgS3Uploader::copyURL);
    connect(m_openUrlButton, &QPushButton::clicked,
            this, &ImgS3Uploader::openURL);
    connect(m_deleteImageOnS3, &QPushButton::clicked,
            this, &ImgS3Uploader::deleteImageOnS3);
    connect(m_toClipboardButton, &QPushButton::clicked,
            this, &ImgS3Uploader::copyImage);
}

void ImgS3Uploader::openURL() {
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgS3Uploader::copyURL() {
    QApplication::clipboard()->setText(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void ImgS3Uploader::copyImage() {
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}

void ImgS3Uploader::deleteImageOnS3() {
    if(nullptr != m_imageLabel) {
        m_imageLabel->hide();
    }
    m_spinner->show();
    setInfoLabelText(tr("Deleting image..."));
    deleteResource(m_s3ImageName, m_deleteToken);
}

bool ImgS3Uploader::success() {
    return m_success;
}

void ImgS3Uploader::hideSpinner() {
    if(nullptr != m_spinner) {
        m_spinner->hide();
    }
    if(nullptr != m_imageLabel) {
        m_imageLabel->hide();
    }
}

void ImgS3Uploader::setInfoLabelText(const QString &infoText) {
    m_infoLabel->setText(infoText);
    m_infoLabel->show();
}
