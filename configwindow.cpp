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

    m_buttonListView = new QListWidget(this);
    m_buttonListView->setFlow(QListWidget::TopToBottom);
    m_buttonListView->setMouseTracking(true);

    QSettings settings;
    m_listButtons = settings.value("buttons").value<QList<int> >();
    initButtonList();

    connect(m_buttonListView, &QListWidget::itemChanged, this,
            &ConfigWindow::updateActiveButtons);

    baseLayout->addWidget(m_buttonListView);
    show();
}
#include <QDebug>
void ConfigWindow::initButtonList() {
    for (int i = 0; i != static_cast<int>(Button::Type::last); ++i) {
        auto t = static_cast<Button::Type>(i);
        // Blocked items
        if (t ==Button::Type::mouseVisibility || t ==Button::Type::text ||
                t ==Button::Type::imageUploader || t ==Button::Type::arrow) {
            continue;
        }
        QListWidgetItem *buttonItem = new QListWidgetItem(m_buttonListView);

        bool iconsAreWhite = false;
        QString bgColor = this->palette().color(QWidget::backgroundRole()).name();
        // when the background is lighter than gray, it uses the white icons
        if (bgColor < QColor(Qt::gray).name()) {
            iconsAreWhite = true;
        }
        buttonItem->setIcon(Button::getIcon(t, iconsAreWhite));

        buttonItem->setText(Button::getTypeName(t));
        buttonItem->setToolTip(Button::getTypeTooltip(t));
        if (m_listButtons.contains(i)) {
            buttonItem->setCheckState(Qt::Checked);
        } else {
            buttonItem->setCheckState(Qt::Unchecked);
        }
    }
}

void ConfigWindow::updateActiveButtons(QListWidgetItem *item) {
    int buttonIndex = static_cast<int>(Button::getTypeByName(item->text()));
    if (item->checkState() == Qt::Checked) {
        m_listButtons.append(buttonIndex);
    } else {
        m_listButtons.removeOne(buttonIndex);
    }
    QSettings().setValue("buttons", QVariant::fromValue(m_listButtons));
}
