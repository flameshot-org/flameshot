#include "historywidget.h"
#include "src/utils/history.h"
#include "src/utils/configenterprise.h"
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
#include <QClipboard>


HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("Screenshots history"));
    setFixedSize(this->size());

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

    foreach(QString fileName, historyFiles) {
        QString fullFileName = history.path() + fileName;

        QPixmap pixmap;
        pixmap.load( fullFileName, "png" );
        pixmap = pixmap.scaledToWidth(120);

        QPushButton *button = new QPushButton;
        button->setStyleSheet("Text-align:left");
        QIcon buttonIcon(fullFileName);
        button->setIcon(buttonIcon);
        button->setIconSize(pixmap.rect().size());

        QFileInfo *pFileInfo = new QFileInfo(fullFileName);
        QString lastModified = pFileInfo->lastModified().toString("yyyy-MM-dd hh:mm:ss");
        button->setText(lastModified);

        connect(button, &QPushButton::clicked, this, [=](){
            // TODO - optimize it
            this->close();
            ConfigEnterprise configEnterprise;
            QSettings *settings = configEnterprise.settings();
            settings->beginGroup("S3");
            QString url = settings->value("S3_URL").toString() + fileName;
            settings->endGroup();
            QApplication::clipboard()->setText(url);
//            qDebug() << "URL copied to clipboard:" << url;

//            NotificationWidget *notification = new NotificationWidget();
//            notification->showMessage(tr("URL copied to clipboard."));
        });


        m_pVBox->addWidget(button);
    }
}
