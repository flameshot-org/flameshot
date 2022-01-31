#include "uploadlineitem.h"
#include "./ui_uploadlineitem.h"
#include "src/core/flameshotdaemon.h"
#include "src/tools/imgupload/imguploadermanager.h"
#include "src/utils/confighandler.h"
#include "src/utils/history.h"
#include "src/widgets/notificationwidget.h"

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

    connect(ui->copyUrl, &QPushButton::clicked, this, [=]() {
        FlameshotDaemon::copyToClipboard(url);
    });

    connect(ui->openBrowser, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl(url));
    });

    connect(ui->deleteImage, &QPushButton::clicked, this, [=]() {
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
        imgUploaderBase->deleteImage(unpackFileName.file, unpackFileName.token);

        removeCacheFile(fullFileName);
        emit requestedDeletion();
    });
}

UploadLineItem::~UploadLineItem()
{
    delete ui;
}