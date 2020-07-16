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

    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleReply);

    m_NetworkAMCreds = new QNetworkAccessManager(this);
    connect(m_NetworkAMCreds, &QNetworkAccessManager::finished, this, &ImgS3Uploader::handleCredsReply);

    setAttribute(Qt::WA_DeleteOnClose);

    // get enterprise settings
    m_configEnterprise = new ConfigEnterprise();
    QSettings *settings = m_configEnterprise->settings();

    // get s3 credentials
    settings->beginGroup("S3");
    m_s3CredsUrl = settings->value("S3_CREDS_URL").toString();
    m_s3XApiKey = settings->value("S3_X_API_KEY").toString();
    settings->endGroup();

    // set proxy server parameters
    QString httpProxyHost = settings->value("HTTP_PROXY_HOST").toString();
    if(httpProxyHost.length() > 0) {
        qDebug() << "Using proxy server";
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
    }

    // set proxy server parameters
    if(httpProxyHost.length() > 0) {
        m_proxy->setHostName(httpProxyHost);
        if(settings->contains("HTTP_PROXY_PORT")) {
            m_proxy->setPort(settings->value("HTTP_PROXY_PORT").toInt());
        } else {
            m_proxy->setPort(3128);
        }

        if(settings->contains("HTTP_PROXY_USER")) {
            m_proxy->setUser(settings->value("HTTP_PROXY_USER").toString());
        }
        if(settings->contains("HTTP_PROXY_PASSWORD")) {
            m_proxy->setPassword(settings->value("HTTP_PROXY_PASSWORD").toString());
        }
        QNetworkProxy::setApplicationProxy(*m_proxy);
        m_NetworkAM->setProxy(*m_proxy);
        m_NetworkAMCreds->setProxy(*m_proxy);
    }

    upload();
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
    QJsonObject json = response.object();
    QJsonObject formData = json["formData"].toObject();
    QJsonObject fields = formData["fields"].toObject();

    QString resultURL = json["resultURL"].toString();

    QString url = formData["url"].toString();

    QString acl = fields["acl"].toString();
    QString contentType = fields["Content-Type"].toString();
    QString key = fields["Key"].toString();
    QString bucket = fields["bucket"].toString();
    QString xAmzAlgorithm = fields["X-Amz-Algorithm"].toString();
    QString xAmzCredential = fields["X-Amz-Credential"].toString();
    QString xAmzDate = fields["X-Amz-Date"].toString();
    QString xAmzSecurityToken = fields["X-Amz-Security-Token"].toString();
    QString policy = fields["Policy"].toString();
    QString xAmzSignature = fields["X-Amz-Signature"].toString();

    //
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart aclPart;
    aclPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"acl\""));
    aclPart.setBody(acl.toLatin1());
    multiPart->append(aclPart);

    QHttpPart contentTypePart;
    contentTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"Content-Type\""));
    contentTypePart.setBody(contentType.toLatin1());
    multiPart->append(contentTypePart);

    QHttpPart keyPart;
    keyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"Key\""));
    keyPart.setBody(key.toLatin1());
    multiPart->append(keyPart);

    QHttpPart bucketPart;
    bucketPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"bucket\""));
    bucketPart.setBody(bucket.toLatin1());
    multiPart->append(bucketPart);

    QHttpPart xAmzAlgorithmPart;
    xAmzAlgorithmPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"X-Amz-Algorithm\""));
    xAmzAlgorithmPart.setBody(xAmzAlgorithm.toLatin1());
    multiPart->append(xAmzAlgorithmPart);

    QHttpPart xAmzCredentialPart;
    xAmzCredentialPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"X-Amz-Credential\""));
    xAmzCredentialPart.setBody(xAmzCredential.toLatin1());
    multiPart->append(xAmzCredentialPart);

    QHttpPart xAmzDatePart;
    xAmzDatePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"X-Amz-Date\""));
    xAmzDatePart.setBody(xAmzDate.toLatin1());
    multiPart->append(xAmzDatePart);

    QHttpPart xAmzSecurityTokenPart;
    xAmzSecurityTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"X-Amz-Security-Token\""));
    xAmzSecurityTokenPart.setBody(xAmzSecurityToken.toLatin1());
    multiPart->append(xAmzSecurityTokenPart);

    QHttpPart policyPart;
    policyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"Policy\""));
    policyPart.setBody(policy.toLatin1());
    multiPart->append(policyPart);

    QHttpPart xAmzSignaturePart;
    xAmzSignaturePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"X-Amz-Signature\""));
    xAmzSignaturePart.setBody(xAmzSignature.toLatin1());
    multiPart->append(xAmzSignaturePart);


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
