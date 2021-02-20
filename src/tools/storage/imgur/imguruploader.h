// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "../imguploader.h"
#include <QUrl>
#include <QWidget>

class QNetworkReply;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class QUrl;
class NotificationWidget;

class ImgurUploader : public ImgUploader
{
    Q_OBJECT
public:
    explicit ImgurUploader(const QPixmap& capture, QWidget* parent = nullptr);
    void upload();

private slots:
    void handleReply(QNetworkReply* reply);

protected slots:
    void deleteImageOnStorage();

    // class members
private:
    QNetworkAccessManager* m_NetworkAM;
    QUrl m_deleteImageURL;
};
