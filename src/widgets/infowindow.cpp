// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021-2022 Jeremy Borgman & Contributors

#include "infowindow.h"
#include "./ui_infowindow.h"
#include "src/core/flameshotdaemon.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/globalvalues.h"
#include <QKeyEvent>
#include <QScreen>

InfoWindow::InfoWindow(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->IconSVG->setPixmap(QPixmap(GlobalValues::iconPath()));
    ui->VersionDetails->setText(GlobalValues::versionInfo());
    ui->OperatingSystemDetails->setText(generateKernelString());

    connect(
      ui->CopyInfoButton, &QPushButton::clicked, this, &InfoWindow::copyInfo);

    show();
    // Call show() first, otherwise the correct geometry cannot be fetched for
    // centering the window on the screen
    QRect position = frameGeometry();
    QScreen* screen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

void InfoWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

QString generateKernelString()
{
    QString kernelVersion =
      QSysInfo::kernelType() + ": " + QSysInfo::kernelVersion() + "\n" +
      QSysInfo::productType() + ": " + QSysInfo::productVersion();
    return kernelVersion;
}

void InfoWindow::copyInfo()
{
    FlameshotDaemon::copyToClipboard(GlobalValues::versionInfo() + "\n" +
                                     generateKernelString());
}
