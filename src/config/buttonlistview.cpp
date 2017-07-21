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

#include "buttonlistview.h"
#include "src/capture/tools/toolfactory.h"
#include "src/utils/confighandler.h"
#include <QListWidgetItem>
#include <QListWidgetItem>
#include <QSettings>
#include <algorithm>

ButtonListView::ButtonListView(QWidget *parent) : QListWidget(parent) {
    setMouseTracking(true);
    setFlow(QListWidget::TopToBottom);
    initButtonList();
    connect(this, &QListWidget::itemChanged, this,
            &ButtonListView::updateActiveButtons);
    connect(this, &QListWidget::itemClicked, this,
            &ButtonListView::reverseItemCheck);
}

void ButtonListView::initButtonList() {
    m_listButtons = QSettings().value("buttons").value<QList<int> >();
    ToolFactory factory;
    auto listTypes = CaptureButton::getIterableButtonTypes();

    for (CaptureButton::ButtonType t: listTypes) {
        CaptureTool *tool = factory.CreateTool(t);

        // add element to the local map
        m_buttonTypeByName.insert(tool->getName(), t);

        // init the menu option

        QListWidgetItem *buttonItem = new QListWidgetItem(this);

        // when the background is lighter than gray, it uses the white icons
        QColor bgColor = this->palette().color(QWidget::backgroundRole());
        QString color = bgColor.valueF() < 0.6 ? "White" : "Black";
        QString iconPath = QString(":/img/buttonIcons%1/%2")
                .arg(color).arg(tool->getIconName());
        buttonItem->setIcon(QIcon(iconPath));

        buttonItem->setFlags(Qt::ItemIsUserCheckable);
        QColor foregroundColor = this->palette().color(QWidget::foregroundRole());
        buttonItem->setTextColor(foregroundColor);

        buttonItem->setText(tool->getName());
        buttonItem->setToolTip(tool->getDescription());
        if (m_listButtons.contains(static_cast<int>(t))) {
            buttonItem->setCheckState(Qt::Checked);
        } else {
            buttonItem->setCheckState(Qt::Unchecked);
        }
        tool->deleteLater();
    }
}

void ButtonListView::updateActiveButtons(QListWidgetItem *item) {
    CaptureButton::ButtonType bType = m_buttonTypeByName[item->text()];
    int buttonIndex = static_cast<int>(bType);

    if (item->checkState() == Qt::Checked) {
        m_listButtons.append(buttonIndex);
        std::sort(m_listButtons.begin(), m_listButtons.end());
    } else {
        m_listButtons.removeOne(buttonIndex);
    }

    QSettings().setValue("buttons", QVariant::fromValue(m_listButtons));
}

void ButtonListView::reverseItemCheck(QListWidgetItem *item){
    if (item->checkState() == Qt::Checked) {
        item->setCheckState(Qt::Unchecked);
    } else {
        item->setCheckState(Qt::Checked);
    }
}

void ButtonListView::selectAll() {
    ConfigHandler().setAllTheButtons();
    for(int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        item->setCheckState(Qt::Checked);
    }
}
