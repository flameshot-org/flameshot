// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "droplruploader.h"
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
#include <QCursor>
#include <QDesktopServices>
#include <QDrag>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

DroplrUploader::DroplrUploader(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
{
    setWindowTitle(tr("Upload to Imgur"));
    setWindowIcon(QIcon(":img/app/flameshot.svg"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiMainColorValue());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Uploading Image"));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &DroplrUploader::handleReply);

    setAttribute(Qt::WA_DeleteOnClose);

    upload();
}

void DroplrUploader::handleReply(QNetworkReply* reply)
{
    m_spinner->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        m_imageURL.setUrl(json[QStringLiteral("shortlink")].toString());

        // Droplr doesn't have dedicated URL for delete.
        m_deleteImageURL.setUrl(json[QStringLiteral("shortlink")].toString());

        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            SystemNotification().sendMessage(
              QObject::tr("URL copied to clipboard."));
            QApplication::clipboard()->setText(m_imageURL.toString());
            close();
        } else {
            onUploadOk();
        }
    } else {
        m_infoLabel->setText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void DroplrUploader::startDrag()
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>{ m_imageURL });
    mimeData->setImageData(m_pixmap);

    QDrag* dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(
      256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void DroplrUploader::upload()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    m_pixmap.save(&buffer, "PNG");

    QUrlQuery urlQuery;
    QString fileName = FileNameHandler().parsedPattern();
    urlQuery.addQueryItem(QStringLiteral("filename"),
                          QStringLiteral("%1.png").arg(fileName).toUtf8());

    QUrl url(QStringLiteral("https://api.droplr.com/files"));
    url.setQuery(urlQuery);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");

    QString droplrUsername = ConfigHandler().droplrUsername();
    QString droplrPassword = ConfigHandler().droplrPassword();
    QString authCredentials = QStringLiteral("%1:%2")
                                .arg(droplrUsername)
                                .arg(droplrPassword)
                                .toUtf8()
                                .toBase64();
    request.setRawHeader(
      "Authorization",
      QStringLiteral("Basic %1").arg(authCredentials).toUtf8());

    m_NetworkAM->post(request, byteArray);
}

void DroplrUploader::onUploadOk()
{
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    ImageLabel* imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(
      imageLabel, &ImageLabel::dragInitiated, this, &DroplrUploader::startDrag);
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

    connect(
      m_copyUrlButton, &QPushButton::clicked, this, &DroplrUploader::copyURL);
    connect(
      m_openUrlButton, &QPushButton::clicked, this, &DroplrUploader::openURL);
    connect(m_openDeleteUrlButton,
            &QPushButton::clicked,
            this,
            &DroplrUploader::openDeleteURL);
    connect(m_toClipboardButton,
            &QPushButton::clicked,
            this,
            &DroplrUploader::copyImage);
}

void DroplrUploader::openURL()
{
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void DroplrUploader::copyURL()
{
    QApplication::clipboard()->setText(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void DroplrUploader::openDeleteURL()
{
    bool successful = QDesktopServices::openUrl(m_deleteImageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void DroplrUploader::copyImage()
{
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}
