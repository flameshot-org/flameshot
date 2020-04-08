// Copyright(c) 2020 Tobias Eliasson <arnestig@gmail.com>
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "historywindow.h"
#include "src/core/controller.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/widgets/imagelabel.h"
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QIcon>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QKeyEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QCursor>
#include <QRect>
#include <QScreen>
#include <QGuiApplication>
#endif

// historywindow shows old snapshots taken and allows to copy utilize them again

HistoryWindow::HistoryWindow(QWidget *parent) : QWidget(parent) {
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowTitle(tr("History"));

    connect(this, &HistoryWindow::captureTaken,
            Controller::getInstance(), &Controller::captureTaken);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignHCenter);
    initTabWidget();
}

void HistoryWindow::initTabWidget() {
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setMinimumWidth(800);
    m_tabWidget->setMinimumHeight(400);
    m_tabWidget->setTabsClosable(true);
    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    m_layout->addWidget(m_tabWidget);

    // Ctrl + W for closing a tab
    QAction *closeAction = new QAction(this);
    closeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
    m_tabWidget->addAction(closeAction);
    connect(closeAction, &QAction::triggered, this, &HistoryWindow::closeTabShortcut);

    // Ctrl + C for copying an image in a tab
    QAction *copyAction = new QAction(this);
    copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    m_tabWidget->addAction(copyAction);
    connect(copyAction, &QAction::triggered, this, &HistoryWindow::copyImage);
}

void HistoryWindow::copyImage()
{
    QWidget *tab = m_tabWidget->currentWidget();
    if ( tab != NULL ) {
        ImageLabel *label = tab->findChild<ImageLabel*>();
        if ( label != NULL ) {
            QApplication::clipboard()->setPixmap(label->getScreenshot());
        }
    }
}

void HistoryWindow::closeTabShortcut()
{
    closeTab(m_tabWidget->currentIndex());
}

void HistoryWindow::closeTab(int index)
{
    QString tabName = m_tabWidget->tabText(index);
    m_historyMap.remove( tabName );
    m_tabWidget->removeTab( index );
}

void HistoryWindow::addImage( QString tabName, QPixmap p )
{
    QWidget *tab;
    ImageLabel *label;

    // save screenshot with date key
    m_historyMap[ tabName ] = p;
    auto ity = m_historyMap.find(tabName);

    // see if we can find a tab with the tabName already in our widget
    tab = m_tabWidget->findChild<QWidget*>( tabName );
    if ( tab != NULL ) {
        label = tab->findChild<ImageLabel*>();
        if ( label != NULL ) {
            label->setScreenshot(ity.value());
        }
    } else {
        tab = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(tab);
        label = new ImageLabel(tab);
        label->setScreenshot(ity.value());
        tab->setObjectName( tabName );

        connect(label, &ImageLabel::historyEdit, this, &HistoryWindow::edit);

        layout->addWidget(label);
        m_tabWidget->addTab(tab, ity.key());
        m_tabWidget->setTabText(m_tabWidget->indexOf(tab), tabName);
        m_tabWidget->setCurrentIndex(m_tabWidget->indexOf(tab));
    }
}

void HistoryWindow::edit()
{
    QString tabName = m_tabWidget->tabText(m_tabWidget->currentIndex());
    QPixmap p = m_historyMap[ tabName ];
    CaptureWidget *captureWidget = new CaptureWidget(m_tabWidget->currentIndex(), QString(),false,p);
    captureWidget->show();
    connect(captureWidget, &CaptureWidget::captureTaken, this, &HistoryWindow::editDone);
}

void HistoryWindow::editDone(uint id, QPixmap p)
{
    QString tabName = m_tabWidget->tabText(id);
    addImage(tabName,p);
}
