// Copyright(c) 2017-2021 Namecheap inc.
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

#include "imgs3settings.h"
#include "src/tools/imgupload/storages/imguploaderbase.h"
#include <QWidget>

#define SCREENSHOT_STORAGE_TYPE_S3 "s3"

class QNetworkReply;
class QNetworkProxy;
class QNetworkAccessManager;
class QHttpMultiPart;
class QSettings;

class ImgS3Uploader : public ImgUploaderBase
{
    Q_OBJECT
public:
    explicit ImgS3Uploader(const QPixmap& capture, QWidget* parent = nullptr);
    explicit ImgS3Uploader(QWidget* parent = nullptr);
    ~ImgS3Uploader();
    void upload();

public slots:
    void deleteImage(const QString& fileName, const QString& deleteToken);

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

    QString m_storageImageName;
    QString m_deleteToken;

    // TODO - wtf
    bool resultStatus;
};
