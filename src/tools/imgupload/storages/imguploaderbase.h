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

class ImgUploaderBase : public QWidget
{
    Q_OBJECT
public:
    explicit ImgUploaderBase(const QPixmap& capture, QWidget* parent = nullptr);

    LoadSpinner* spinner();

    const QUrl& imageURL();
    void setImageURL(const QUrl&);
    const QPixmap& pixmap();
    void setPixmap(const QPixmap&);
    void setInfoLabelText(const QString&);

    NotificationWidget* notification();
    virtual void deleteImage(const QString& fileName,
                             const QString& deleteToken) = 0;
    virtual void upload() = 0;

signals:
    void uploadOk(const QUrl& url);
    void deleteOk();

public slots:
    void showPostUploadDialog();

private slots:
    void startDrag();
    void openURL();
    void copyURL();
    void copyImage();
    void deleteCurrentImage();
    void saveScreenshotToFilesystem();

private:
    QPixmap m_pixmap;

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
    QPushButton* m_saveToFilesystemButton;
    QUrl m_imageURL;
    NotificationWidget* m_notification;

public:
    QString m_currentImageName;
};
