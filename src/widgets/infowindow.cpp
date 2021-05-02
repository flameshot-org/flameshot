// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "infowindow.h"
#include "src/core/qguiappcurrentscreen.h"
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
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
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
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
    QLabel* icon = new QLabel();
    icon->setPixmap(QPixmap(":img/app/flameshot.svg"));
    icon->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(icon);

    QLabel* licenseTitleLabel = new QLabel(tr("<u><b>License</b></u>"), this);
    licenseTitleLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(licenseTitleLabel);

    QLabel* licenseLabel = new QLabel(QStringLiteral("GPLv3+"), this);
    licenseLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(licenseLabel);
    m_layout->addStretch();

    QLabel* versionTitleLabel = new QLabel(tr("<u><b>Version</b></u>"), this);
    versionTitleLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(versionTitleLabel);
    QString versionMsg = "Flameshot " + QStringLiteral(APP_VERSION) + " (" +
                         QStringLiteral(FLAMESHOT_GIT_HASH) +
                         ")\nCompiled with Qt " + QT_VERSION_STR;
    QLabel* versionLabel = new QLabel(versionMsg, this);
    versionLabel->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(versionLabel);
    m_layout->addSpacing(30);
}

void InfoWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }
}
