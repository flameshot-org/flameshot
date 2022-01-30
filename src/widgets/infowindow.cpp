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

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    show();
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

void InfoWindow::on_CopyInfoButton_clicked()
{
    FlameshotDaemon::copyToClipboard(GlobalValues::versionInfo() + "\n" +
                                     generateKernelString());
}
