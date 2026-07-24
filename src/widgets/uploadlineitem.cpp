#include "uploadlineitem.h"
#include "./ui_uploadlineitem.h"
#include "core/flameshotdaemon.h"
#include "tools/imgupload/imguploadermanager.h"
#include "tools/imgupload/storages/imguploaderbase.h"
#include "utils/confighandler.h"
#include "utils/history.h"
#include "widgets/notificationwidget.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>
#include <QWidget>

void removeCacheFile(QString const& fullFileName)
{
    QFile file(fullFileName);
    if (file.exists()) {
        file.remove();
    }
}

UploadLineItem::UploadLineItem(QWidget* parent,
                               QPixmap const& preview,
                               QString const& timestamp,
                               QString const& url,
                               QString const& fullFileName,
                               HistoryFileName const& unpackFileName)
  : QWidget(parent)
  , ui(new Ui::UploadLineItem)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->imagePreview->setPixmap(preview);
    ui->uploadTimestamp->setText(timestamp);

    connect(ui->copyUrl, &QPushButton::clicked, this, [=, this]() {
        FlameshotDaemon::copyToClipboard(url);
    });

    connect(ui->openBrowser, &QPushButton::clicked, this, [=, this]() {
        QDesktopServices::openUrl(QUrl(url));
    });

    connect(ui->deleteImage, &QPushButton::clicked, this, [=, this]() {
        if (ConfigHandler().historyConfirmationToDelete() &&
            QMessageBox::No ==
              QMessageBox::question(
                this,
                tr("Confirm to delete"),
                tr("Are you sure you want to delete a screenshot from the "
                   "latest uploads and server?"),
                QMessageBox::Yes | QMessageBox::No)) {
            return;
        }

        ImgUploaderBase* imgUploaderBase =
          ImgUploaderManager(this).uploader(unpackFileName.type);

        // Drop the local history entry only once the backend confirms the
        // remote delete. Imgur emits deleteOk synchronously (unchanged
        // behavior); an async API delete (Drive) may instead report failure,
        // in which case the entry is kept and the user is told.
        connect(imgUploaderBase, &ImgUploaderBase::deleteOk, this, [=, this]() {
            removeCacheFile(fullFileName);
            emit requestedDeletion();
        });
        connect(imgUploaderBase,
                &ImgUploaderBase::deleteFail,
                this,
                [this](const QString& error) {
                    QMessageBox::warning(
                      this,
                      tr("Unable to delete screenshot"),
                      error.isEmpty()
                        ? tr("The screenshot could not be deleted "
                             "from the server.")
                        : error);
                });

        imgUploaderBase->deleteImage(unpackFileName.file, unpackFileName.token);
    });
}

UploadLineItem::~UploadLineItem()
{
    delete ui;
}
