// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

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

class ImgurUploader : public QWidget
{
    Q_OBJECT
public:
    explicit ImgurUploader(const QPixmap& capture, QWidget* parent = nullptr);

private slots:
    void handleReply(QNetworkReply* reply);
    void startDrag();

    void openURL();
    void copyURL();
    void openDeleteURL();
    void copyImage();

private:
    QPixmap m_pixmap;
    QNetworkAccessManager* m_NetworkAM;

    QVBoxLayout* m_vLayout;
    QHBoxLayout* m_hLayout;
    // loading
    QLabel* m_infoLabel;
    LoadSpinner* m_spinner;
    // uploaded
    QPushButton* m_openUrlButton;
    QPushButton* m_openDeleteUrlButton;
    QPushButton* m_copyUrlButton;
    QPushButton* m_toClipboardButton;
    QUrl m_imageURL;
    QUrl m_deleteImageURL;
    NotificationWidget* m_notification;

    void upload();
    void onUploadOk();
};
