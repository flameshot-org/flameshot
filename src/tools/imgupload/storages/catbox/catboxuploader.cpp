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
#include <cstdio>
#include <ios>
#include <qstringliteral.h>
#include <qurlquery.h>
#include <qvariant.h>

CatboxUploader::CatboxUploader(const QPixmap& capture, QWidget* parent)
  : ImgUploaderBase(capture, parent)
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
        QString url = reply->readAll();
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
          "catbox", ConfigHandler().catboxUserHash(), m_currentImageName);
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

    QUrl url(QStringLiteral(CATBOX_API_URL));
    QHttpMultiPart *http = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart reqTypePart;
    reqTypePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    reqTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"reqtype\""));
    reqTypePart.setBody("fileupload");
    http->append(reqTypePart);

    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userhash\""));
    userPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    userPart.setBody(ConfigHandler().catboxUserHash().toUtf8());
    http->append(userPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"fileToUpload\"; filename=\"upload.png\""));
    filePart.setBody(byteArray);
    http->append(filePart);

    QNetworkRequest request(url);
    request.setRawHeader("Cookie",
                         QStringLiteral("PHPSESSID %1")
                           .arg(ConfigHandler().catboxUserHash())
                           .toUtf8());

    QNetworkReply* reply = m_NetworkAM->post(request, http);
    http->setParent(reply);
}

void CatboxUploader::deleteImage(const QString& fileName,
                                 const QString& deleteToken)
{
    Q_UNUSED(fileName)
    m_NetworkAM = new QNetworkAccessManager(this);

    QUrl url(QStringLiteral(CATBOX_API_URL));
    QHttpMultiPart *http = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart reqTypePart;
    reqTypePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    reqTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"reqtype\""));
    reqTypePart.setBody("deletefiles");
    http->append(reqTypePart);

    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userhash\""));
    userPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    userPart.setBody(deleteToken.toUtf8());
    http->append(userPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"files\";"));
    filePart.setBody(fileName.toUtf8());
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    http->append(filePart);

    QNetworkRequest request(url);
    request.setRawHeader("Cookie",
                         QStringLiteral("PHPSESSID %1")
                           .arg(ConfigHandler().catboxUserHash())
                           .toUtf8());

    QNetworkReply* reply = m_NetworkAM->post(request, http);
    http->setParent(reply);

    if (reply->error() != QNetworkReply::NoError) {
        notification()->showMessage(tr("Unable to delete file."));
    }

    emit deleteOk();
}
