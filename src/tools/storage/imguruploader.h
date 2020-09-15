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

#include "imguploader.h"
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
    //    void openDeleteURL();

protected slots:
    void deleteImageOnStorage();

    // class members
private:
    QNetworkAccessManager* m_NetworkAM;
    QUrl m_deleteImageURL;
};
