// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "selectiontexttool.h"
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QTimer>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QStringList>

#include <tesseract/baseapi.h>

SelectionTextTool::SelectionTextTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SelectionTextTool::closeOnButtonPressed() const
{
    return true;
}

QIcon SelectionTextTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "selectiontexttool.svg");
}

QString SelectionTextTool::name() const
{
    return tr("Recognize Text (OCR)");
}

CaptureTool::Type SelectionTextTool::type() const
{
    return CaptureTool::TYPE_SELECTION_TEXT;
}

QString SelectionTextTool::description() const
{
    return tr("Select an area to copy text to clipboard");
}

CaptureTool* SelectionTextTool::copy(QObject* parent)
{
    return new SelectionTextTool(parent);
}

static QString defaultTessdataPath()
{
#ifdef Q_OS_WIN
    const QStringList candidates = {
        qEnvironmentVariable("TESSDATA_PREFIX"),
        QCoreApplication::applicationDirPath() + "/tessdata",
        "C:/Program Files/Tesseract-OCR/tessdata",
        "C:/Program Files (x86)/Tesseract-OCR/tessdata"
    };
#elif defined(Q_OS_MACOS)
    const QStringList candidates = {
        qEnvironmentVariable("TESSDATA_PREFIX"),
        QCoreApplication::applicationDirPath() + "/tessdata",
        "/usr/local/share/tessdata",
        "/opt/homebrew/share/tessdata",
        "/usr/share/tessdata"
    };
#else
    const QStringList candidates = {
        qEnvironmentVariable("TESSDATA_PREFIX"),
        QCoreApplication::applicationDirPath() + "/tessdata",
        "/usr/share/tesseract-ocr/4.00/tessdata",
        "/usr/share/tessdata",
        "/usr/local/share/tessdata"
    };
#endif

    for (const QString& p : candidates) {
        if (p.isEmpty())
            continue;
        QDir d(p);
        if (d.exists())
            return d.absolutePath();
    }
    return QString();
}

static QString gatherLanguages(const QString& tessdataPath)
{
    if (tessdataPath.isEmpty())
        return QString();

    QDir dir(tessdataPath);
    QStringList entries = dir.entryList(QStringList() << "*.traineddata", QDir::Files);
    if (entries.isEmpty())
        return QString();

    QStringList langs;
    langs.reserve(entries.size());
    for (const QString& e : entries) {
        QString base = QFileInfo(e).baseName();
        if (!base.isEmpty())
            langs << base;
    }
    langs.removeDuplicates();
    return langs.join('+');
}

void SelectionTextTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);

    const QRect selection = context.selection;
    if (!selection.isValid())
        return;

    const QPixmap croppedPixmap = context.origScreenshot.copy(selection);
    QImage image = croppedPixmap.toImage().convertToFormat(QImage::Format_RGB888);

    QString tessdataPath = defaultTessdataPath();
    QString langString = gatherLanguages(tessdataPath);
    if (langString.isEmpty())
        langString = "eng";

    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init(tessdataPath.isEmpty() ? nullptr : tessdataPath.toUtf8().constData(),
                  langString.toUtf8().constData())) {
        delete api;
        qDebug() << "Tesseract initialization failed for" << tessdataPath << "langs:" << langString;
        return;
    }

    api->SetImage(image.bits(), image.width(), image.height(), 3, image.bytesPerLine());

    char* outText = api->GetUTF8Text();
    QString recognizedText = QString::fromUtf8(outText ? outText : "").trimmed();

    delete[] outText;
    api->End();
    delete api;

    if (!recognizedText.isEmpty()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(recognizedText, QClipboard::Clipboard);
#ifdef Q_OS_UNIX
        clipboard->setText(recognizedText, QClipboard::Selection);
#endif
        QApplication::processEvents();

        // Delay to allow clipboard managers (especially on Wayland) to take ownership
#ifdef Q_OS_LINUX
        if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
            QProcess wl;
            wl.start("wl-copy");
            if (wl.waitForStarted(300)) {
                wl.write(recognizedText.toUtf8());
                wl.closeWriteChannel();
                wl.waitForFinished(500);
            }
            QTimer::singleShot(150, this, [this]() {
                emit requestAction(REQ_CAPTURE_DONE_OK);
                emit requestAction(REQ_CLOSE_GUI);
            });
            return;
        }
#endif
        QTimer::singleShot(50, this, [this]() {
            emit requestAction(REQ_CAPTURE_DONE_OK);
            emit requestAction(REQ_CLOSE_GUI);
        });
    } else {
        QTimer::singleShot(50, this, [this]() {
            emit requestAction(REQ_CAPTURE_DONE_OK);
            emit requestAction(REQ_CLOSE_GUI);
        });
    }
}
