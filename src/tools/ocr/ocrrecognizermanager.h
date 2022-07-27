// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#pragma once

#include "src/tools/ocr/services/ocrrecognizerbase.h"
#include <QObject>

#define OCR_RECOGNIZER_SERVICES_DEFAULT "server"

class QPixmap;
class QWidget;

class OcrRecognizerManager : public QObject
{
    Q_OBJECT
public:
    explicit OcrRecognizerManager(QObject* parent = nullptr);

    OcrRecognizerBase* recognizer(const QPixmap& capture,
                              QWidget* parent = nullptr);
    OcrRecognizerBase* recognizer(const QString& ocrRecognizerPlugin);

    const QString& ocrRecognizerPlugin();

private:
    void init();

private:
    OcrRecognizerBase* m_ocrRecognizerBase;
    QString m_ocrRecognizerPlugin;
};
