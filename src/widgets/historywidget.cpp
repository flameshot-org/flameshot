#include "historywidget.h"
#include "src/utils/history.h"
#include "src/utils/configenterprise.h"
#include "src/widgets/notificationwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QLabel>
#include <QScrollArea>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QPushButton>
#include <QIcon>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QClipboard>
#include <QUrl>


HistoryWidget::HistoryWidget(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Screenshots history"));
    setFixedSize(800, this->height());
    m_notification = new NotificationWidget();

    m_pVBox = new QVBoxLayout(this);
    m_pVBox->setAlignment(Qt::AlignTop);

    QScrollArea *scrollArea = new QScrollArea( this );
    scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry( this->frameGeometry() );

    QWidget *widget = new QWidget();
    scrollArea->setWidget( widget );
    widget->setLayout( m_pVBox );

    loadHistory();
}

void HistoryWidget::loadHistory() {
    History history = History();
    QList<QString> historyFiles = history.history();

    ConfigEnterprise configEnterprise;
    QSettings *settings = configEnterprise.settings();
    settings->beginGroup("S3");
    QString s3BaseUrl = settings->value("S3_URL").toString();
    settings->endGroup();

    if(historyFiles.isEmpty()) {
        QPushButton *buttonEmpty = new QPushButton;
        buttonEmpty->setText(tr("Screenshots history is empty"));
        buttonEmpty->setMinimumSize(1, HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
        connect(buttonEmpty, &QPushButton::clicked, this, [=](){
            this->close();
        });
        m_pVBox->addWidget(buttonEmpty);
        return;
    }
    foreach(QString fileName, historyFiles) {
        // generate url
        QString fullFileName = history.path() + fileName;
        QString url = s3BaseUrl + fileName;

        // load pixmap
        QPixmap pixmap;
        pixmap.load( fullFileName, "png" );

        if (pixmap.height() / HISTORYPIXMAP_MAX_PREVIEW_HEIGHT >= pixmap.width() / HISTORYPIXMAP_MAX_PREVIEW_WIDTH) {
            pixmap = pixmap.scaledToHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
        } else {
            pixmap = pixmap.scaledToWidth(HISTORYPIXMAP_MAX_PREVIEW_WIDTH);
        }

        // get file info
        QFileInfo *pFileInfo = new QFileInfo(fullFileName);
        QString lastModified = pFileInfo->lastModified().toString(" yyyy-MM-dd hh:mm:ss");

        // screenshot preview
        QLabel *pScreenshot = new QLabel();
        pScreenshot->setStyleSheet("padding: 5px;");
        pScreenshot->setMinimumHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
        pScreenshot->setPixmap(pixmap);

        // screenshot datetime
        QLabel *pScreenshotText = new QLabel();
        pScreenshotText->setStyleSheet("padding: 5px;");
        pScreenshotText->setMinimumHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
        pScreenshotText->setText(lastModified);

        // copy url
        QPushButton *buttonCopyUrl = new QPushButton;
        buttonCopyUrl->setText(tr("Copy URL"));
        buttonCopyUrl->setMinimumHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
        connect(buttonCopyUrl, &QPushButton::clicked, this, [=](){
            QApplication::clipboard()->setText(url);
            m_notification->showMessage(tr("URL copied to clipboard."));
            this->close();
        });

        // open in browser
        QPushButton *buttonOpen = new QPushButton;
        buttonOpen->setText(tr("Open in browser"));
        buttonOpen->setMinimumHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
        connect(buttonOpen, &QPushButton::clicked, this, [=](){
            QDesktopServices::openUrl(QUrl(url));
            this->close();
        });

        // layout
        QHBoxLayout *phbl = new QHBoxLayout();
        phbl->addWidget(pScreenshot);
        phbl->addWidget(pScreenshotText);
        phbl->addWidget(buttonCopyUrl);
        phbl->addWidget(buttonOpen);

        phbl->setStretchFactor(pScreenshot, 3);
        phbl->setStretchFactor(pScreenshotText, 2);
        phbl->setStretchFactor(buttonCopyUrl, 2);
        phbl->setStretchFactor(buttonOpen, 2);

        // add to scroll
        m_pVBox->addLayout(phbl);
    }
}
