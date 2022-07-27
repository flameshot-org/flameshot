// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Cavivie & Contributors

#include "ocrrecognizerbase.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/utils/history.h"
#include "src/utils/screenshotsaver.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QApplication>
// FIXME #include <QBuffer>
#include <QClipboard>
#include <QCursor>
#include <QDesktopServices>
#include <QDrag>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QShortcut>
#include <QTextEdit>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

OcrRecognizerBase::OcrRecognizerBase(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
{
    setWindowTitle(tr("OCR Recognizer"));
    setWindowIcon(QIcon(GlobalValues::iconPath()));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiColor());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Recognizing Image"));
    m_infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_infoLabel->setCursor(QCursor(Qt::IBeamCursor));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    setAttribute(Qt::WA_DeleteOnClose);
}

LoadSpinner* OcrRecognizerBase::spinner()
{
    return m_spinner;
}

const QString& OcrRecognizerBase::recognizedText()
{
    return m_recognizedText;
}

void OcrRecognizerBase::setRecognizedText(const QString& recognizedText)
{
    m_recognizedText = recognizedText;
}

const QPixmap& OcrRecognizerBase::pixmap()
{
    return m_pixmap;
}

void OcrRecognizerBase::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
}

NotificationWidget* OcrRecognizerBase::notification()
{
    return m_notification;
}

void OcrRecognizerBase::setInfoLabelText(const QString& text)
{
    m_infoLabel->setText(text);
}

void OcrRecognizerBase::startDrag()
{
    auto* mimeData = new QMimeData;
    mimeData->setText(m_recognizedText);
    mimeData->setImageData(m_pixmap);

    auto* dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(
      256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void OcrRecognizerBase::showPostRecognizeDialog()
{
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    auto* imageLabel = new ImageLabel();
    imageLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    imageLabel->setScreenshot(m_pixmap);
    connect(imageLabel,
            &ImageLabel::dragInitiated,
            this,
            &OcrRecognizerBase::startDrag);
    m_vLayout->addWidget(imageLabel);

    auto* ocrTextBox = new QTextEdit();
    ocrTextBox->setText(recognizedText());
    m_vLayout->addWidget(ocrTextBox);

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyTextButton = new QPushButton(tr("Copy Text"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_saveToFilesystemButton = new QPushButton(tr("Save image"));
    m_hLayout->addWidget(m_copyTextButton);
    m_hLayout->addWidget(m_toClipboardButton);
    m_hLayout->addWidget(m_saveToFilesystemButton);

    connect(m_copyTextButton,
            &QPushButton::clicked,
            this,
            &OcrRecognizerBase::copyText);
    connect(m_toClipboardButton,
            &QPushButton::clicked,
            this,
            &OcrRecognizerBase::copyImage);

    QObject::connect(m_saveToFilesystemButton,
                     &QPushButton::clicked,
                     this,
                     &OcrRecognizerBase::saveScreenshotToFilesystem);
}

void OcrRecognizerBase::copyText()
{
    FlameshotDaemon::copyToClipboard(m_recognizedText);
    m_notification->showMessage(tr("Recognized text copied to clipboard."));
}

void OcrRecognizerBase::copyImage()
{
    FlameshotDaemon::copyToClipboard(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}

void OcrRecognizerBase::saveScreenshotToFilesystem()
{
    if (!saveToFilesystemGUI(m_pixmap)) {
        m_notification->showMessage(
          tr("Unable to save the screenshot to disk."));
        return;
    }
    m_notification->showMessage(tr("Screenshot saved."));
}
