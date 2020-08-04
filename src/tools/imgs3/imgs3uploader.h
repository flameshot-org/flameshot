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

class ImgS3Uploader : public QWidget {
    Q_OBJECT
public:
    explicit ImgS3Uploader(const QPixmap &capture, QWidget *parent = nullptr);

private slots:
    void handleReply(QNetworkReply *reply);
    void handleCredsReply(QNetworkReply *reply);
    void startDrag();

    void openURL();
    void copyURL();
    void copyImage();

private:
    void uploadToS3(QJsonDocument &response);
    void initNetwork();

private:
    ConfigEnterprise *m_configEnterprise;
    QString m_s3CredsUrl;
    QString m_s3XApiKey;

    QString m_hostName;
    QPixmap m_pixmap;
    QNetworkProxy *m_proxy;
    QNetworkAccessManager *m_NetworkAM;
    QNetworkAccessManager *m_NetworkAMCreds;

    QVBoxLayout *m_vLayout;
    QHBoxLayout *m_hLayout;
    // loading
    QLabel *m_infoLabel;
    LoadSpinner *m_spinner;
    // uploaded
    QPushButton *m_openUrlButton;
    QPushButton *m_copyUrlButton;
    QPushButton *m_toClipboardButton;
    QUrl m_imageURL;
    NotificationWidget *m_notification;

    void upload();
    void onUploadOk();
};
