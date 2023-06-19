// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "catboxuploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QBuffer>
#include <QDesktopServices>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QUrlQuery>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qstringliteral.h>
#include <qurlquery.h>

CatboxUploader::CatboxUploader(const QPixmax& capture, QWidget* parent)
  : ImgurUploader(capture, parent)
{
    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &CatboxUploader::handleReply);
}

void CatboxUploader::handleReply(QNetworkReply* reply)
{
    spinner()->deleteLater();
    m_currentImageName.clear();
    if (reply->error() == QNetworkReply::NoError) {
        QStringLiteral url = QStringLiteral(reply->readAll());
        setImageURL(url);

        // save history
        m_currentImageName = imageURL().toString();
        int lastSlash = m_currentImageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            m_currentImageName = m_currentImageName.mid(lastSlash + 1);
        }

        // save image to history
        History history;
        m_currentImageName = history.packFileName(
          "catbox", imageURL().toString(), m_currentImageName);
        history.save(pixmap(), m_currentImageName);

        emit uploadOk(imageURL());
    } else {
        setInfoLabelText(reply->errorString());
    }

    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void CatboxUploader::upload()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap().save(&buffer, "PNG");

    QHttpMultiPart* multiPart =
      new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QUrlQuery params;
    params.addQueryItem("reqtype", "fileupload");
    params.addQueryItem("fileToUpload", FileNameHandler().parsedPattern());
    params.addQueryItem("userhash", ConfigHandler().uploadClientSecret());
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data;"));
    imagePart.setBodyDevice(buffer);

    multiPart.append(params);
    multiPart.append(imagePart);

    QUrl url(QStringLiteral("https://catbox.moe/api.php"));
    QNetworkRequest request(url);
    request.setHeader(workRequest::ContentTypeHeader, "multipart/form-data");

    request.setRawHeader("Cookie",
                         QStringLiteral("PHPSESSID %1")
                           .arg(ConfigHandler().uploadClientSecret())
                           .toUtf8());

    reply = m_NetworkAM->post(request, multiPart);
    multiPart->setParent(reply);
}

void CatboxUploader::deleteImage(const QString& fileName,
                                 const QString& deleteToken)
{
    Q_UNUSED(fileName)
    m_NetworkAM = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://catbox.moe/api.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject postData;
    postData.insert("reqtype", "deletefiles");
    postData.insert("files", fileName);
    postData.insert("userhash", deleteToken);

    QNetworkReply* reply = manager.post(
      request, QJsonDocument(postData).toJson(QJsonDocument::Compact));

    if (reply->error() != QNetworkReply::NoError) {
        notification()->showMessage(tr("Unable to delete file."));
    }

    emit deleteOk();
}
