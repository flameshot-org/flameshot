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

#include "imgstorages.h"
#include "s3/imgs3settings.h"
#include <QUrl>
#include <QWidget>

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
class ImageLabel;

class ImgUploader : public QWidget
{
    Q_OBJECT
public:
    explicit ImgUploader(const QPixmap& capture, QWidget* parent = nullptr);
    explicit ImgUploader(QWidget* parent = nullptr);

    virtual void upload(){};
    virtual void deleteResource(const QString&, const QString&){};

protected:
    const QPixmap& pixmap();
    void setImageUrl(const QUrl&);
    const QUrl& imageUrl();
    void onUploadOk();
    void hideSpinner();
    void setInfoLabelText(const QString&);
    void showNotificationMessage(const QString&);

public slots:
    virtual void openURL();
    virtual void copyURL();
    virtual void copyImage();
    virtual void deleteImageOnStorage();
    virtual void startDrag();

private:
    void init(const QString& title, const QString& label);

    // class members
private:
    QPixmap m_pixmap;

    QUrl m_imageURL;
    ImageLabel* m_imageLabel;

    NotificationWidget* m_notification;

    QVBoxLayout* m_vLayout;
    QHBoxLayout* m_hLayout;
    // loading
    LoadSpinner* m_spinner;
    QLabel* m_infoLabel;
    // uploaded
    QPushButton* m_openUrlButton;
    QPushButton* m_copyUrlButton;
    QPushButton* m_toClipboardButton;
    QPushButton* m_deleteImageOnStorage;

protected:
    //    bool m_success;

    // Temporary variables
    QString m_deleteToken;
    QString m_storageImageName;

public:
    bool resultStatus;
};
