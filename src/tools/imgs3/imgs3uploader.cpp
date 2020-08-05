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
    m_proxy = nullptr;
    setWindowTitle(tr("Upload to ImgS3"));
    setWindowIcon(QIcon(":img/app/flameshot.svg"));

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiMainColorValue());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Uploading Image"));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    setAttribute(Qt::WA_DeleteOnClose);

    // get enterprise settings
    m_configEnterprise = new ConfigEnterprise();

    // get s3 credentials
    QSettings *settings = m_configEnterprise->settings();
    settings->beginGroup("S3");
    m_s3CredsUrl = settings->value("S3_CREDS_URL").toString();
    m_s3XApiKey = settings->value("S3_X_API_KEY").toString();
    settings->endGroup();

    initNetwork();
    upload();
}

void ImgS3Uploader::initNetwork() {
    // Init network
    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleReply);

    m_NetworkAMCreds = new QNetworkAccessManager(this);
    connect(m_NetworkAMCreds, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleCredsReply);

    // get proxy settings from "config.ini" file
    QSettings *settings = m_configEnterprise->settings();
    QString httpProxyHost = settings->value("HTTP_PROXY_HOST").toString();
    if(httpProxyHost.length() > 0 && m_proxy == nullptr) {
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
        QNetworkProxyQuery q(QUrl(m_s3CredsUrl.toUtf8()));
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

    if(m_proxy != nullptr) {
        qDebug() << "Using proxy server";
        qDebug() << "proxy host:" << m_proxy->hostName();
        qDebug() << "proxy port:" << m_proxy->port();
        qDebug() << "proxy type:" << m_proxy->type();
        qDebug() << "proxy user:" << (m_proxy->user().length() > 0 ? m_proxy->user() : "no user");
        qDebug() << "proxy password:" << (m_proxy->password().length() > 0 ? "***" : "no password");


        QNetworkProxy::setApplicationProxy(*m_proxy);
        m_NetworkAM->setProxy(*m_proxy);
        m_NetworkAMCreds->setProxy(*m_proxy);
    }
    else {
        qDebug() << "No proxy";
    }
}


void ImgS3Uploader::handleReply(QNetworkReply *reply) {
    m_spinner->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        // save history
        QString imageName = m_imageURL.toString();
        int lastSlash = imageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            imageName = imageName.mid(lastSlash);
        }
        History history;
        history.save(m_pixmap, imageName);

        // Copy url to clipboard if required
        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            QApplication::clipboard()->setText(m_imageURL.toString());
            SystemNotification().sendMessage(QObject::tr("URL copied to clipboard."));
            close();
        } else {
            onUploadOk();
        }
    } else {
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        m_infoLabel->setText(reply->errorString());
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

void ImgS3Uploader::handleCredsReply(QNetworkReply *reply){
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        uploadToS3(response);
    } else {
        if(m_s3CredsUrl.length() == 0){
            m_infoLabel->setText("S3 Creds URL is not found in your configuration file");
        }
        else {
            m_infoLabel->setText(reply->errorString());
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
    m_NetworkAM->post(request, multiPart);
}

void ImgS3Uploader::upload() {
    // get creads
    QUrl creds(m_s3CredsUrl);
    QNetworkRequest requestCreds(creds);
    if(m_s3XApiKey.length() > 0) {
        requestCreds.setRawHeader(QByteArray("X-API-Key"), QByteArray(m_s3XApiKey.toLocal8Bit()));
    }
    m_NetworkAMCreds->get(requestCreds);
}

void ImgS3Uploader::onUploadOk() {
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    ImageLabel *imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(imageLabel, &ImageLabel::dragInitiated, this, &ImgS3Uploader::startDrag);
    m_vLayout->addWidget(imageLabel);

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_openUrlButton);
    m_hLayout->addWidget(m_toClipboardButton);

    connect(m_copyUrlButton, &QPushButton::clicked,
            this, &ImgS3Uploader::copyURL);
    connect(m_openUrlButton, &QPushButton::clicked,
            this, &ImgS3Uploader::openURL);
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
