// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#pragma once

#include <QWidget>

class QNetworkReply;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class NotificationWidget;

class OcrRecognizerBase : public QWidget
{
    Q_OBJECT
public:
    explicit OcrRecognizerBase(const QPixmap& capture,
                               QWidget* parent = nullptr);

    LoadSpinner* spinner();

    const QString& recognizedText();
    void setRecognizedText(const QString&);
    const QPixmap& pixmap();
    void setPixmap(const QPixmap&);
    void setInfoLabelText(const QString&);

    NotificationWidget* notification();
    virtual void recognize() = 0;

signals:
    void recognizedOk(const QString& text);

public slots:
    void showPostRecognizeDialog();

private slots:
    void startDrag();
    void copyText();
    void copyImage();
    void saveScreenshotToFilesystem();

private:
    QPixmap m_pixmap;

    QVBoxLayout* m_vLayout;
    QHBoxLayout* m_hLayout;
    // loading
    QLabel* m_infoLabel;
    LoadSpinner* m_spinner;
    // recognized
    QPushButton* m_copyTextButton;
    QPushButton* m_toClipboardButton;
    QPushButton* m_saveToFilesystemButton;
    QString m_recognizedText;
    NotificationWidget* m_notification;

public:
};
