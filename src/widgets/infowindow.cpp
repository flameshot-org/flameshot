// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "infowindow.h"
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QCursor>
#include <QGuiApplication>
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
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
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
    QString versionMsg = "Flameshot " + QStringLiteral(APP_VERSION) +
                         "\nCompiled with Qt " + QT_VERSION_STR;
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
