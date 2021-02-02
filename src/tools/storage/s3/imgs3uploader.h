// Copyright(c) 2017-2019 Namecheap inc.
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

#pragma once

#include "../imguploader.h"
#include "imgs3settings.h"
#include <QUrl>
#include <QWidget>

class QNetworkReply;
class QNetworkProxy;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QUrl;
class NotificationWidget;
class ImageLabel;
class QHttpMultiPart;

class ImgS3Uploader : public ImgUploader
{
    Q_OBJECT
public:
    explicit ImgS3Uploader(const QPixmap& capture, QWidget* parent = nullptr);
    explicit ImgS3Uploader(QWidget* parent = nullptr);
    ~ImgS3Uploader();
    void upload();
    void deleteResource(const QString&, const QString&);

private slots:
    void handleReplyPostUpload(QNetworkReply* reply);
    void handleReplyGetCreds(QNetworkReply* reply);
    void handleReplyDeleteResource(QNetworkReply* reply);
    void handleReplyGetConfig(QNetworkReply* reply);

private:
    void init(const QString& title, const QString& label);
    void uploadToS3(QJsonDocument& response);
    void removeImagePreview();
    void getConfigRemote();

    void clearProxy();
    QNetworkProxy* proxy();

    void onUploadError(QNetworkReply* reply);

    // class members
private:
    ImgS3Settings m_s3Settings;

    QNetworkAccessManager* m_networkAMUpload;
    QNetworkAccessManager* m_networkAMGetCreds;
    QNetworkAccessManager* m_networkAMRemove;
    QHttpMultiPart* m_multiPart;
    QNetworkAccessManager* m_networkAMConfig;
};
