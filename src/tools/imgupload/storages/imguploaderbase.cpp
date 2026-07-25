// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguploaderbase.h"
#include "core/flameshotdaemon.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"
#include "utils/history.h"
#include "utils/screenshotsaver.h"
#include "widgets/imagelabel.h"
#include "widgets/loadspinner.h"
#include "widgets/notificationwidget.h"

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
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

ImgUploaderBase::ImgUploaderBase(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
  , m_notification(nullptr)
{
    setWindowTitle(tr("Upload image"));
    setWindowIcon(QIcon(GlobalValues::iconPath()));

    QRect position = frameGeometry();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiColor());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Uploading Image"));
    m_infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_infoLabel->setCursor(QCursor(Qt::IBeamCursor));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    setAttribute(Qt::WA_DeleteOnClose);
}

LoadSpinner* ImgUploaderBase::spinner()
{
    return m_spinner;
}

const QUrl& ImgUploaderBase::imageURL()
{
    return m_imageURL;
}

void ImgUploaderBase::setImageURL(const QUrl& imageURL)
{
    m_imageURL = imageURL;
}

const QPixmap& ImgUploaderBase::pixmap()
{
    return m_pixmap;
}

void ImgUploaderBase::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
}

NotificationWidget* ImgUploaderBase::notification()
{
    return m_notification;
}

void ImgUploaderBase::setInfoLabelText(const QString& text)
{
    m_infoLabel->setText(text);
}

void ImgUploaderBase::startDrag()
{
    auto* mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>{ m_imageURL });
    mimeData->setImageData(m_pixmap);

    auto* dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(
      256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void ImgUploaderBase::showPostUploadDialog()
{
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    auto* imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(imageLabel,
            &ImageLabel::dragInitiated,
            this,
            &ImgUploaderBase::startDrag);
    m_vLayout->addWidget(imageLabel);

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_openDeleteUrlButton = new QPushButton(tr("Delete image"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_saveToFilesystemButton = new QPushButton(tr("Save image"));
    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_openUrlButton);
    m_hLayout->addWidget(m_openDeleteUrlButton);
    m_hLayout->addWidget(m_toClipboardButton);
    m_hLayout->addWidget(m_saveToFilesystemButton);

    connect(
      m_copyUrlButton, &QPushButton::clicked, this, &ImgUploaderBase::copyURL);
    connect(
      m_openUrlButton, &QPushButton::clicked, this, &ImgUploaderBase::openURL);
    connect(m_openDeleteUrlButton,
            &QPushButton::clicked,
            this,
            &ImgUploaderBase::deleteCurrentImage);
    connect(m_toClipboardButton,
            &QPushButton::clicked,
            this,
            &ImgUploaderBase::copyImage);

    QObject::connect(m_saveToFilesystemButton,
                     &QPushButton::clicked,
                     this,
                     &ImgUploaderBase::saveScreenshotToFilesystem);

    // The delete outcomes are wired here, where the delete control is born, so
    // each connection lives exactly as long as the control it serves. Wiring
    // them inside the delete slot instead would stack a fresh pair on every
    // retry after a failure, making the number of removal attempts depend on
    // the user's click history (KTD2).
    connect(this, &ImgUploaderBase::deleteOk, this, [this]() {
        // The remote file is gone, so drop the local history entry through the
        // one removal path both delete controls share. Gated on the backend's
        // confirmation, never optimistic: a delete that fails keeps a
        // recoverable entry (KD1).
        History().remove(m_currentImageName);
    });
    connect(
      this, &ImgUploaderBase::deleteFail, this, [this](const QString& error) {
          // Backend-neutral failure feedback: every backend and every
          // failure cause reports here, including the Drive
          // authorization paths that were silent before (KTD4).
          m_openDeleteUrlButton->setEnabled(true);
          if (m_notification) {
              m_notification->showMessage(
                error.isEmpty() ? tr("The screenshot could not be deleted.")
                                : error);
          }
      });
}

void ImgUploaderBase::openURL()
{
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgUploaderBase::copyURL()
{
    FlameshotDaemon::copyToClipboard(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void ImgUploaderBase::copyImage()
{
    FlameshotDaemon::copyToClipboard(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}

void ImgUploaderBase::deleteCurrentImage()
{
    // A delete can be asynchronous, so disable the control before dispatching:
    // a second click must not start a parallel delete (R3). The failure handler
    // re-enables it; on success the file is gone and it stays disabled.
    m_openDeleteUrlButton->setEnabled(false);

    History history;
    HistoryFileName unpackFileName = history.unpackFileName(m_currentImageName);
    deleteImage(unpackFileName.file, unpackFileName.token);
}

void ImgUploaderBase::saveScreenshotToFilesystem()
{
    if (!saveToFilesystemGUI(m_pixmap)) {
        m_notification->showMessage(
          tr("Unable to save the screenshot to disk."));
        return;
    }
    m_notification->showMessage(tr("Screenshot saved."));
}
