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
#include "src/core/controller.h"
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
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QUrlQuery>
#include <QVBoxLayout>

ImgurUploader::ImgurUploader(const QPixmap& capture, QWidget* parent)
  : ImgUploader(capture, parent)
{
    setWindowTitle(tr("Upload to Imgur"));

    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &ImgurUploader::handleReply);
}

void ImgurUploader::handleReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        QJsonObject data = json[QStringLiteral("data")].toObject();
        setImageUrl(data[QStringLiteral("link")].toString());
        m_deleteToken = data[QStringLiteral("deletehash")].toString();

        m_deleteImageURL.setUrl(
          QStringLiteral("https://imgur.com/delete/%1").arg(m_deleteToken));

        // save history
        QString imageName = imageUrl().toString();
        int lastSlash = imageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            imageName = imageName.mid(lastSlash + 1);
        }
        m_storageImageName = imageName;

        // save image to history
        History history;
        imageName = history.packFileName(
          SCREENSHOT_STORAGE_TYPE_IMGUR, m_deleteToken, imageName);
        history.save(pixmap(), imageName);
        resultStatus = true;

        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            QApplication::clipboard()->setText(imageUrl().toString());
            SystemNotification().sendMessage(
              QObject::tr("URL copied to clipboard."));
            Controller::getInstance()->updateRecentScreenshots();
            close();
        } else {
            onUploadOk();
        }
    } else {
        setInfoLabelText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgurUploader::upload()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap().save(&buffer, "PNG");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("title"),
                          QStringLiteral("flameshot_screenshot"));
    QString description = FileNameHandler().parsedPattern();
    urlQuery.addQueryItem(QStringLiteral("description"), description);

    QUrl url(QStringLiteral("https://api.imgur.com/3/image"));
    url.setQuery(urlQuery);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/application/x-www-form-urlencoded");
    request.setRawHeader(
      "Authorization",
      QStringLiteral("Client-ID %1").arg(IMGUR_CLIENT_ID).toUtf8());

    m_NetworkAM->post(request, byteArray);
}

void ImgurUploader::deleteImageOnStorage()
{
    bool successful = QDesktopServices::openUrl(m_deleteImageURL);
    if (!successful) {
        showNotificationMessage(tr("Unable to open the URL."));
    }
}
