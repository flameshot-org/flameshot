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


HistoryWidget::HistoryWidget(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Screenshots history"));
    setFixedSize(this->size());
    m_notification = new NotificationWidget();

    m_pVBox = new QVBoxLayout(this);

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

    foreach(QString fileName, historyFiles) {
        // generate url
        QString fullFileName = history.path() + fileName;
        QString url = s3BaseUrl + fileName;

        // load pixmap
        QPixmap pixmap;
        pixmap.load( fullFileName, "png" );
        pixmap = pixmap.scaledToWidth(HISTORYWIDGET_MAX_PREVIEW_HEIGHT);

        // get file info
        QFileInfo *pFileInfo = new QFileInfo(fullFileName);
        QString lastModified = pFileInfo->lastModified().toString(" yyyy-MM-dd hh:mm:ss");

        // copy url
        QPushButton *buttonCopyUrl = new QPushButton;
        buttonCopyUrl->setStyleSheet("text-align:left");
        QIcon buttonIcon(fullFileName);
        buttonCopyUrl->setIcon(buttonIcon);
        buttonCopyUrl->setStyleSheet("padding: 5px;");
        buttonCopyUrl->setIconSize(pixmap.rect().size());
        buttonCopyUrl->setText(lastModified);
        connect(buttonCopyUrl, &QPushButton::clicked, this, [=](){
            QApplication::clipboard()->setText(url);
            m_notification->showMessage(tr("URL copied to clipboard."));
            this->close();
        });

        // open in browser
        QPushButton *buttonOpen = new QPushButton;
        buttonOpen->setText(tr("Open in browser"));
        connect(buttonOpen, &QPushButton::clicked, this, [=](){
            QDesktopServices::openUrl(QUrl(url));
            this->close();
        });

        // layout
        QHBoxLayout *phbl = new QHBoxLayout();
        phbl->addWidget(buttonCopyUrl);
        phbl->addWidget(buttonOpen);

        int nHeight = pixmap.rect().size().height() >= HISTORYWIDGET_MAX_PREVIEW_HEIGHT
                ? HISTORYWIDGET_MAX_PREVIEW_HEIGHT
                : pixmap.rect().size().height();
        buttonOpen->setMinimumSize(1, nHeight + 5 * 2 + 2); // padding 5px*2 + border
        phbl->setStretchFactor(buttonCopyUrl, 7);
        phbl->setStretchFactor(buttonOpen, 3);

        // add to scroll
        m_pVBox->addLayout(phbl);
    }
}
