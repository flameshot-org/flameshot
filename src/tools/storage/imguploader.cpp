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

#include "imguploader.h"
#include "src/utils/configenterprise.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QDrag>
#include <QHBoxLayout>
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

ImgUploader::ImgUploader(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
{
    init(tr("Upload image to S3"), tr("Uploading Image"));
}

ImgUploader::ImgUploader(QWidget* parent)
  : QWidget(parent)
{
    init(tr("Upload image"), tr("Uploading Image"));
}

void ImgUploader::init(const QString& title, const QString& label)
{
    m_imageLabel = nullptr;
    m_spinner = nullptr;

    resultStatus = false;
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

void ImgUploader::openURL()
{
    bool successful = QDesktopServices::openUrl(imageUrl());
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgUploader::copyURL()
{
    QApplication::clipboard()->setText(imageUrl().toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void ImgUploader::copyImage()
{
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}

void ImgUploader::deleteImageOnStorage()
{
    if (nullptr != m_imageLabel) {
        m_imageLabel->hide();
    }
    m_spinner->show();
    setInfoLabelText(tr("Deleting image..."));
    deleteResource(m_storageImageName, m_deleteToken);
}

void ImgUploader::startDrag()
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>{ imageUrl() });
    mimeData->setImageData(m_pixmap);

    QDrag* dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(
      256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void ImgUploader::hideSpinner()
{
    if (nullptr != m_spinner) {
        m_spinner->hide();
    }
    if (nullptr != m_imageLabel) {
        m_imageLabel->hide();
    }
}

void ImgUploader::setInfoLabelText(const QString& infoText)
{
    m_infoLabel->setText(infoText);
    m_infoLabel->show();
}

const QPixmap& ImgUploader::pixmap()
{
    return m_pixmap;
}

void ImgUploader::onUploadOk()
{
    hideSpinner();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    if (nullptr == m_imageLabel) {
        m_imageLabel = new ImageLabel();
        m_imageLabel->setScreenshot(pixmap());
        m_imageLabel->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);
        connect(m_imageLabel,
                &ImageLabel::dragInitiated,
                this,
                &ImgUploader::startDrag);
        m_vLayout->addWidget(m_imageLabel);
    }

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_deleteImageOnStorage = new QPushButton(tr("Delete image"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_openUrlButton);
    m_hLayout->addWidget(m_deleteImageOnStorage);
    m_hLayout->addWidget(m_toClipboardButton);

    connect(
      m_copyUrlButton, &QPushButton::clicked, this, &ImgUploader::copyURL);
    connect(
      m_openUrlButton, &QPushButton::clicked, this, &ImgUploader::openURL);
    connect(m_deleteImageOnStorage,
            &QPushButton::clicked,
            this,
            &ImgUploader::deleteImageOnStorage);
    connect(m_toClipboardButton,
            &QPushButton::clicked,
            this,
            &ImgUploader::copyImage);
}

void ImgUploader::setImageUrl(const QUrl& imageURL)
{
    m_imageURL = imageURL;
}
const QUrl& ImgUploader::imageUrl()
{
    return m_imageURL;
}

void ImgUploader::showNotificationMessage(const QString& notificationMessage) {
    m_notification->showMessage(notificationMessage);
}

