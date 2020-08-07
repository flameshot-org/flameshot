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

#pragma once

#define S3_API_IMG_PATH "v2/image/"

#include "imgs3settings.h"
#include <QWidget>
#include <QUrl>

class QNetworkReply;
class QNetworkProxy;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class QUrl;
class NotificationWidget;
class ConfigEnterprise;
class ImageLabel;

class ImgS3Uploader : public QWidget {
    Q_OBJECT
public:
    explicit ImgS3Uploader(const QPixmap &capture, QWidget *parent = nullptr);
    explicit ImgS3Uploader(QWidget *parent = nullptr);
    void upload();
    void deleteResource(const QString &, const QString &);
    bool success();

private slots:
    void handleReplyUpload(QNetworkReply *reply);
    void handleReplyGetCreds(QNetworkReply *reply);
    void handleReplyDeleteResource(QNetworkReply *reply);
    void startDrag();

    void openURL();
    void copyURL();
    void copyImage();
    void deleteImageOnS3();

private:
    void init(const QString &title, const QString &label);
    void uploadToS3(QJsonDocument &response);
    void initNetwork();

    void onUploadOk();

    void hideSpinner();
    void setInfoLabelText(const QString &);


// class members
private:
    bool m_success;
    ConfigEnterprise *m_configEnterprise;
    ImgS3Settings m_s3Settings;

    ImageLabel *m_imageLabel;

    QString m_hostName;
    QPixmap m_pixmap;
    QNetworkProxy *m_proxy;
    QNetworkAccessManager *m_NetworkAMUpload;
    QNetworkAccessManager *m_NetworkAMGetCreds;
    QNetworkAccessManager *m_NetworkAMRemove;

    QVBoxLayout *m_vLayout;
    QHBoxLayout *m_hLayout;
    // loading
    QLabel *m_infoLabel;
    LoadSpinner *m_spinner;
    // uploaded
    QPushButton *m_openUrlButton;
    QPushButton *m_copyUrlButton;
    QPushButton *m_toClipboardButton;
    QPushButton *m_deleteImageOnS3;
    QUrl m_imageURL;
    NotificationWidget *m_notification;

    // Temporary variables
    QString m_deleteToken;
    QString m_s3ImageName;
};
