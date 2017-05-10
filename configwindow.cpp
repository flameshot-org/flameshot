#include "configwindow.h"
#include "capture/button.h"
#include <QSettings>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QLabel>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget *parent) : QWidget(parent){
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":img/flameshot.svg"));
    setWindowTitle(tr("Configuration"));

    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *buttonSelectLabel = new QLabel("Choose the buttons to enable", this);
    baseLayout->addWidget(buttonSelectLabel);

    QListWidget *buttonList = new QListWidget(this);
    baseLayout->addWidget(buttonList);

    buttonList->setFlow(QListWidget::TopToBottom);
    buttonList->setMouseTracking(true);
    //buttonList->setSelectionMode(QAbstractItemView::NoSelection);
    // http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlistview

    for (int i = 0; i != static_cast<int>(Button::Type::last); ++i) {
        auto t = static_cast<Button::Type>(i);
        QListWidgetItem *buttonItem = new QListWidgetItem(buttonList);
        buttonItem->setIcon(Button::getIcon(t));
        buttonItem->setText(Button::typeName[t]);
        buttonItem->setToolTip(Button::typeTooltip[t]);
        buttonItem->setCheckState(Qt::Unchecked);
    }

    QSettings settings;
    // http://www.qtcentre.org/threads/52957-QColor-vector-in-QSettings

    // black white icons
    // color interface

    // buttons available

    // path save
    // color paint tools
    show();
}
