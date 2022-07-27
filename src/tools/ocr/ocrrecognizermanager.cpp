// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#include "ocrrecognizermanager.h"
#include <QPixmap>
#include <QWidget>

// TODO - remove this hard-code and create plugin manager in the future, you may
// include other recognizer service headers here
#include "services/server/serverrecognizer.h"

OcrRecognizerManager::OcrRecognizerManager(QObject* parent)
  : QObject(parent)
  , m_ocrRecognizerBase(nullptr)
{
    // TODO - implement OcrRecognizer for other recognizer services and
    // selection among them
    m_ocrRecognizerPlugin = OCR_RECOGNIZER_SERVICES_DEFAULT;
    init();
}

void OcrRecognizerManager::init()
{
    // TODO - implement OcrRecognizer for other recognizer services and
    // selection among them
}

OcrRecognizerBase* OcrRecognizerManager::recognizer(const QPixmap& capture,
                                                    QWidget* parent)
{
    // TODO - implement OcrRecognizer for other recognizer services and
    // selection among them
    m_ocrRecognizerBase =
      (OcrRecognizerBase*)(new ServerRecognizer(capture, parent));
    if (m_ocrRecognizerBase && !capture.isNull()) {
        m_ocrRecognizerBase->recognize();
    }
    return m_ocrRecognizerBase;
}

OcrRecognizerBase* OcrRecognizerManager::recognizer(
  const QString& ocrRecognizerPlugin)
{
    m_ocrRecognizerPlugin = ocrRecognizerPlugin;
    init();
    return recognizer(QPixmap());
}

const QString& OcrRecognizerManager::ocrRecognizerPlugin()
{
    return m_ocrRecognizerPlugin;
}
