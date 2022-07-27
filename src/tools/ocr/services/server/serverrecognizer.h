// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#pragma once

#include "src/tools/ocr/services/ocrrecognizerbase.h"
#include <QUrl>
#include <QWidget>

class QNetworkReply;
class QNetworkAccessManager;
class QUrl;

class ServerRecognizer : public OcrRecognizerBase
{
    Q_OBJECT
public:
    explicit ServerRecognizer(const QPixmap& capture, QWidget* parent = nullptr);

private slots:
    void handleReply(QNetworkReply* reply);

private:
    void recognize();

private:
    QNetworkAccessManager* m_NetworkAM;
};
