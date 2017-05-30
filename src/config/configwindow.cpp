// Copyright 2017 Alejandro Sirgo Rica
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

#include "configwindow.h"
#include "src/capture/button.h"
#include "src/config/buttonlistview.h"
#include "src/config/uicoloreditor.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>

// ConfigWindow contains the menus where you can configure the application

ConfigWindow::ConfigWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(400, 400);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setWindowIcon(QIcon(":img/flameshot.svg"));
    setWindowTitle(tr("Configuration"));

    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *colorSelectionLabel = new QLabel(tr("UI color editor"), this);
    baseLayout->addWidget(colorSelectionLabel);

    baseLayout->addWidget(new UIcolorEditor(this));

    QLabel *buttonSelectLabel = new QLabel(tr("Button selection"), this);
    baseLayout->addWidget(buttonSelectLabel);

    ButtonListView *m_buttonListView = new ButtonListView(this);
    m_buttonListView->setFlow(QListWidget::TopToBottom);
    m_buttonListView->setWhatsThis(tr("Select which buttons will appear arround "
                                   "the capture's selection by clicking on its"
                                   " checkbox."));

    baseLayout->addWidget(m_buttonListView);

    show();
}

void ConfigWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
            close();
    }
}
