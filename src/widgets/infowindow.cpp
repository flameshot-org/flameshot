// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "infowindow.h"
#include "src/core/flameshotdaemon.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/globalvalues.h"
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSysInfo>
#include <QVBoxLayout>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))

#include <QRect>
#include <QScreen>

#endif

// InfoWindow show basic information about the usage of Flameshot

InfoWindow::InfoWindow(QWidget* parent)
  : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowTitle(tr("About"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignHCenter);
    initLabels();
    show();
}

void InfoWindow::initLabels()
{
    auto* icon = new QLabel();
    icon->setPixmap(QPixmap(GlobalValues::iconPath()));
    icon->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(icon);

    auto* licenseTitleLabel = new QLabel(tr("<u><b>License</b></u>"), this);
    licenseTitleLabel->setAlignment(Qt::AlignHCenter);
    licenseTitleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_layout->addWidget(licenseTitleLabel);

    auto* licenseLabel = new QLabel(QStringLiteral("GPLv3+"), this);
    licenseLabel->setAlignment(Qt::AlignHCenter);
    licenseLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_layout->addWidget(licenseLabel);
    m_layout->addStretch();

    auto* versionTitleLabel = new QLabel(tr("<u><b>Version</b></u>"), this);
    versionTitleLabel->setAlignment(Qt::AlignHCenter);
    versionTitleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_layout->addWidget(versionTitleLabel);

    auto* versionLabel = new QLabel(GlobalValues::versionInfo(), this);
    versionLabel->setAlignment(Qt::AlignHCenter);
    versionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    versionLabel->setCursor(QCursor(Qt::IBeamCursor));
    m_layout->addWidget(versionLabel);

    QString kernelInfo = generateKernelString();
    auto* kernelLabel = new QLabel(kernelInfo, this);
    kernelLabel->setAlignment(Qt::AlignHCenter);
    kernelLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    kernelLabel->setCursor(QCursor(Qt::IBeamCursor));
    m_layout->addWidget(kernelLabel);

    auto* copyVersion = new QPushButton("Copy Info", this);
    m_layout->addWidget(copyVersion);
    connect(copyVersion, &QPushButton::pressed, this, &InfoWindow::copyInfo);

    m_layout->addSpacing(30);
}

void InfoWindow::copyInfo()
{
    FlameshotDaemon::copyToClipboard(GlobalValues::versionInfo() + "\n" +
                                     generateKernelString());
}

void InfoWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
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
