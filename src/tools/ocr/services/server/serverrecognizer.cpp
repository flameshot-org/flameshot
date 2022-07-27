// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#include "serverrecognizer.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QBuffer>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QUrlQuery>

ServerRecognizer::ServerRecognizer(const QPixmap& capture, QWidget* parent)
  : OcrRecognizerBase(capture, parent)
{
    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &ServerRecognizer::handleReply);
}

void ServerRecognizer::handleReply(QNetworkReply* reply)
{
    spinner()->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray rawData = reply->readAll();
        QJsonDocument response = QJsonDocument::fromJson(rawData);
        QJsonObject json = response.object();
        if(!json.contains(QStringLiteral("data"))) {
            setInfoLabelText(QStringLiteral("Error response from server: %1").
                             arg(rawData.constData()));
        } else {
            QString data = json[QStringLiteral("data")].toString();
            setRecognizedText(data);

            emit recognizedOk(recognizedText());
        }
    } else {
        setInfoLabelText(reply->errorString());
    }
    FINISH:
        new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ServerRecognizer::recognize()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap().save(&buffer, "PNG");

    QUrl url(ConfigHandler().ocrRecognizerServerUrl());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/application/x-www-form-urlencoded");
    request.setRawHeader("Authorization",
                         ConfigHandler().ocrRecognizerClientKey().toUtf8());

    m_NetworkAM->post(request, byteArray);
}

