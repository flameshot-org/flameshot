// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class QUrl;
class NotificationWidget;

class ImgurUploader : public QWidget
{
    Q_OBJECT
public:
    explicit ImgurUploader(const QPixmap &p, QWidget *parent = nullptr);

private slots:
    void handleReply(QNetworkReply *reply);
    void startDrag();

    void openURL();
    void copyURL();
    void copyImage();

private:
    QPixmap m_pixmap;
    QNetworkAccessManager *m_NetworkAM;

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
