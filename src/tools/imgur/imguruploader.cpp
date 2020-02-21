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

#include "imguruploader.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/notificationwidget.h"
#include "src/utils/confighandler.h"
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
#include <QHttpMultiPart>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

ImgurUploader::ImgurUploader(const QPixmap &capture, QWidget *parent) :
    QWidget(parent), m_pixmap(capture)
{
    QUrl url = ConfigHandler().uploadUrlValue();
    setWindowTitle(tr("Upload to %1").arg(url.host()));
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
    connect(m_NetworkAM, &QNetworkAccessManager::finished, this,
            &ImgurUploader::handleReply);

    setAttribute(Qt::WA_DeleteOnClose);

    upload();
    // QTimer::singleShot(2000, this, &ImgurUploader::onUploadOk); // testing
}

void ImgurUploader::handleReply(QNetworkReply *reply) {
    m_spinner->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        QJsonObject data = json[QStringLiteral("data")].toObject();
        m_imageURL.setUrl(data[QStringLiteral("link")].toString());
        m_deleteImageURL.setUrl(QStringLiteral("https://imgur.com/delete/%1").arg(
                                    data[QStringLiteral("deletehash")].toString()));
        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            QApplication::clipboard()->setText(m_imageURL.toString());
            SystemNotification().sendMessage(QObject::tr("URL copied to clipboard."));
            close();
        } else {
            onUploadOk();
        }
    } else {
        m_infoLabel->setText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgurUploader::startDrag() {
    QMimeData *mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl> { m_imageURL });
    mimeData->setImageData(m_pixmap);

    QDrag *dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(256, 256, Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation));
    dragHandler->exec();
}

void ImgurUploader::upload() {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    m_pixmap.save(&buffer, "PNG");

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
    titlePart.setBody("flameshot_screenshot");

    QHttpPart descPart;
    QString desc = FileNameHandler().parsedPattern();
    descPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"description\""));
    descPart.setBody(desc.toLatin1());

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\"; filename=\"" + desc.toLatin1() + "\""));
    imagePart.setBody(byteArray);

    multiPart->append(titlePart);
    multiPart->append(descPart);
    multiPart->append(imagePart);

    QUrl url = ConfigHandler().uploadUrlValue();
    QNetworkRequest request(url);
    if (!ConfigHandler().isCustomHosting()) {
        request.setRawHeader("Authorization", QStringLiteral("Client-ID %1").arg(IMGUR_CLIENT_ID).toUtf8());
    }

    m_NetworkAM->post(request, multiPart);
}

void ImgurUploader::onUploadOk() {
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    ImageLabel *imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(imageLabel, &ImageLabel::dragInitiated, this, &ImgurUploader::startDrag);
    m_vLayout->addWidget(imageLabel);

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_openDeleteUrlButton = new QPushButton(tr("Delete image"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_openUrlButton);
    m_hLayout->addWidget(m_openDeleteUrlButton);
    m_hLayout->addWidget(m_toClipboardButton);

    if (ConfigHandler().isCustomHosting()) {
        m_openDeleteUrlButton->setEnabled(false);
    }

    connect(m_copyUrlButton, &QPushButton::clicked,
            this, &ImgurUploader::copyURL);
    connect(m_openUrlButton, &QPushButton::clicked,
            this, &ImgurUploader::openURL);
    connect(m_openDeleteUrlButton, &QPushButton::clicked,
            this, &ImgurUploader::openDeleteURL);
    connect(m_toClipboardButton, &QPushButton::clicked,
            this, &ImgurUploader::copyImage);
}

void ImgurUploader::openURL() {
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgurUploader::copyURL() {
    QApplication::clipboard()->setText(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void ImgurUploader::openDeleteURL()
{
    bool successful = QDesktopServices::openUrl(m_deleteImageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgurUploader::copyImage() {
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}
