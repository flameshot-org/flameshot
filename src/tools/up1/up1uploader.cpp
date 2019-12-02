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

#include "up1uploader.h"
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
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

Up1Uploader::Up1Uploader(const QPixmap &capture, QWidget *parent) :
    QWidget(parent), m_pixmap(capture)
{
    setWindowTitle(tr("Upload to Up1"));
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
            &Up1Uploader::handleReply);

    setAttribute(Qt::WA_DeleteOnClose);

    upload();
    // QTimer::singleShot(2000, this, &Up1Uploader::onUploadOk); // testing
}

void Up1Uploader::handleReply(QNetworkReply *reply) {
    delete m_uploadForm;

    m_spinner->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        m_imageURL.setUrl(QStringLiteral(UP1_HOST) + QStringLiteral("/#%1").arg(m_seed));
        m_deleteImageURL.setUrl(QStringLiteral(UP1_HOST) +
                                QStringLiteral("/del?ident=%1&delkey=%2")
                                    .arg(m_ident, json[QStringLiteral("delkey")].toString()));
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

void Up1Uploader::startDrag() {
    QMimeData *mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl> { m_imageURL });
    mimeData->setImageData(m_pixmap);

    QDrag *dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(256, 256, Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation));
    dragHandler->exec();
}

void Up1Uploader::encrypt(QByteArray* input, QByteArray* output, QString& seed, QString& ident) {
    // N.B. IV for CCM is 13 bytes and we require a 64-bit tag for Up1.
    constexpr int IV_LENGTH = 13;
    constexpr int TAG_LENGTH = 8;

    unsigned char entropy[32], hash[64], key[32], iv[16];
    int length, encryptSize;
    EVP_CIPHER_CTX *ctx;
    SHA512_CTX sha512;

    // Metadata + Input
    // This contains:
    // - JSON string of "{ mime: image/png, name: image.png }"
    // - String metadata converted to UTF-16 in big endian order.
    // - Finally appended with separator bytes [0, 0]
    input->prepend(QByteArray().fromBase64("AHsAIgBtAGkAbQBlACIAOgAiAGkAbQBhAGcAZQAvA"
                                           "HAAbgBnACIALAAiAG4AYQBtAGUAIgA6ACIAaQBtAG"
                                           "EAZwBlAC4AcABuAGcAIgB9AAA="));

    // Generate random input to convert to a seed
    RAND_bytes(entropy, sizeof(entropy));

    // The seed can be of any length but must be in URL-encoded Base64.
    seed = QByteArray(reinterpret_cast<const char*>(entropy), 32).toBase64();
    seed.replace("+","-"); seed.replace("/", "_"); seed.replace("=", "");

    // SHA-512 of the seed in base64 produces the encryption keys.
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, entropy, sizeof(entropy));
    SHA512_Final(hash, &sha512);

    // Key = 256 bits, IV = 128 bits
    memcpy(key, hash, 32);
    memcpy(iv, hash + 32, 16);

    // Identity = 128 Bits (URL-Encoded Base64)
    ident = QByteArray(reinterpret_cast<const char*>(hash + 48), 16).toBase64();
    ident.replace("+","-"); ident.replace("/", "_"); ident.replace("=", "");

    // Initialize AES-512 in CCM mode.
    ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ccm(), nullptr, nullptr, nullptr) != 1)
        goto cleanup;

    // Set IV and tag length.
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, IV_LENGTH, nullptr) != 1 ||
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, TAG_LENGTH, nullptr) != 1)
        goto cleanup;

    // Set key material used.
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) 
        goto cleanup;

    // Retrieve the size necessary for the output buffer.
    if (EVP_EncryptUpdate(ctx, nullptr, &length, nullptr,
                          static_cast<int>(input->size())) != 1) 
        goto cleanup;

    // Resize the buffer to the required size + tag length to append.
    encryptSize = length;
    output->resize(length + TAG_LENGTH);

    // Encrypt the full buffer to the destination.
    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(output->data()),
                          &length,
                          reinterpret_cast<const unsigned char*>(input->data()),
                          static_cast<int>(input->size())) != 1)
        goto cleanup;

    // Finalize encryption and append tag.
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(output->data()) + encryptSize,
                            &length) != 1)
        goto cleanup;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_GET_TAG, TAG_LENGTH,
                        reinterpret_cast<unsigned char*>(output->data()) + encryptSize);

cleanup:

    EVP_CIPHER_CTX_cleanup(ctx);
    EVP_CIPHER_CTX_free(ctx);
}

void Up1Uploader::upload() {
    QHttpPart apiKeyPart, identPart, filePart;
    QByteArray input, output;

    QBuffer buffer(&input);
    m_pixmap.save(&buffer, "PNG");

    encrypt(&input, &output, m_seed, m_ident);

    m_uploadForm = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    apiKeyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"api_key\""));
    apiKeyPart.setBody(UP1_API_KEY);

    identPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"ident\""));
    identPart.setBody(QByteArray(m_ident.toLocal8Bit()));

    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"blob\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setBody(output);

    QUrl url(QStringLiteral(UP1_HOST) + QStringLiteral("/up"));
    QNetworkRequest request(url);

    m_uploadForm->append(apiKeyPart);
    m_uploadForm->append(identPart);
    m_uploadForm->append(filePart);

    m_NetworkAM->post(request, m_uploadForm);
}

void Up1Uploader::onUploadOk() {
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    ImageLabel *imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(imageLabel, &ImageLabel::dragInitiated, this, &Up1Uploader::startDrag);
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

    connect(m_copyUrlButton, &QPushButton::clicked,
            this, &Up1Uploader::copyURL);
    connect(m_openUrlButton, &QPushButton::clicked,
            this, &Up1Uploader::openURL);
    connect(m_openDeleteUrlButton, &QPushButton::clicked,
            this, &Up1Uploader::openDeleteURL);
    connect(m_toClipboardButton, &QPushButton::clicked,
            this, &Up1Uploader::copyImage);
}

void Up1Uploader::openURL() {
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void Up1Uploader::copyURL() {
    QApplication::clipboard()->setText(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void Up1Uploader::openDeleteURL()
{
    bool successful = QDesktopServices::openUrl(m_deleteImageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void Up1Uploader::copyImage() {
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}
