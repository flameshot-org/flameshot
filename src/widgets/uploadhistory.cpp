#include "uploadhistory.h"
#include "./ui_uploadhistory.h"
#include "src/tools/imgupload/imguploadermanager.h"
#include "src/utils/confighandler.h"
#include "src/utils/history.h"
#include "uploadlineitem.h"

#include <QDateTime>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QPixmap>

void scaleThumbnail(QPixmap& pixmap)
{
    if (pixmap.height() / HISTORYPIXMAP_MAX_PREVIEW_HEIGHT >=
        pixmap.width() / HISTORYPIXMAP_MAX_PREVIEW_WIDTH) {
        pixmap = pixmap.scaledToHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT,
                                       Qt::SmoothTransformation);
    } else {
        pixmap = pixmap.scaledToWidth(HISTORYPIXMAP_MAX_PREVIEW_WIDTH,
                                      Qt::SmoothTransformation);
    }
}

void clearHistoryLayout(QLayout* layout)
{
    while (layout->count() != 0) {
        delete layout->takeAt(0);
    }
}

UploadHistory::UploadHistory(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::UploadHistory)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
}

void UploadHistory::loadHistory()
{
    clearHistoryLayout(ui->historyContainer);

    History history = History();
    QList<QString> historyFiles = history.history();

    if (historyFiles.isEmpty()) {
        setEmptyMessage();
    } else {
        foreach (QString fileName, historyFiles) {
            addLine(history.path(), fileName);
        }
    }
}

void UploadHistory::setEmptyMessage()
{
    auto* buttonEmpty = new QPushButton;
    buttonEmpty->setText(tr("Screenshots history is empty"));
    buttonEmpty->setMinimumSize(1, HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
    connect(buttonEmpty, &QPushButton::clicked, this, [=]() { this->close(); });
    ui->historyContainer->addWidget(buttonEmpty);
}

void UploadHistory::addLine(const QString& path, const QString& fileName)
{
    QString fullFileName = path + fileName;

    History history;
    HistoryFileName unpackFileName = history.unpackFileName(fileName);

    QString url = ImgUploaderManager(this).url() + unpackFileName.file;

    // load pixmap
    QPixmap pixmap;
    pixmap.load(fullFileName, "png");
    scaleThumbnail(pixmap);

    // get file info
    auto fileInfo = QFileInfo(fullFileName);
    QString lastModified =
      fileInfo.lastModified().toString("yyyy-MM-dd\nhh:mm:ss");

    auto* line = new UploadLineItem(
      this, pixmap, lastModified, url, fullFileName, unpackFileName);

    connect(line, &UploadLineItem::requestedDeletion, this, [=]() {
        if (ui->historyContainer->count() <= 1) {
            setEmptyMessage();
        }
        delete line;
    });

    ui->historyContainer->addWidget(line);
}

UploadHistory::~UploadHistory()
{
    delete ui;
}
