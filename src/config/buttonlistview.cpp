// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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
#include <algorithm>

ButtonListView::ButtonListView(QWidget *parent) : QListWidget(parent) {
    setMouseTracking(true);
    setFlow(QListWidget::TopToBottom);
    initButtonList();
    updateComponents();
    connect(this, &QListWidget::itemClicked, this,
            &ButtonListView::reverseItemCheck);
}

void ButtonListView::initButtonList() {
    ToolFactory factory;
    auto listTypes = CaptureButton::getIterableButtonTypes();

    for (const CaptureButton::ButtonType t: listTypes) {
        CaptureTool *tool = factory.CreateTool(t);

        // add element to the local map
        m_buttonTypeByName.insert(tool->name(), t);

        // init the menu option
        QListWidgetItem *m_buttonItem = new QListWidgetItem(this);

        // when the background is lighter than gray, it uses the white icons
        QColor bgColor = this->palette().color(QWidget::backgroundRole());
        QString color = bgColor.valueF() < 0.6 ? "White" : "Black";
        QString iconPath = QStringLiteral(":/img/buttonIcons%1/%2")
                .arg(color).arg(tool->iconName());
        if (t == CaptureButton::TYPE_SELECTIONINDICATOR) {
            iconPath = QStringLiteral(":/img/buttonIcons%1/size_indicator.png")
                    .arg(color);
        }
        m_buttonItem->setIcon(QIcon(iconPath));

        m_buttonItem->setFlags(Qt::ItemIsUserCheckable);
        QColor foregroundColor = this->palette().color(QWidget::foregroundRole());
        m_buttonItem->setForeground(foregroundColor);

        m_buttonItem->setText(tool->name());
        m_buttonItem->setToolTip(tool->description());
        tool->deleteLater();
    }
}

void ButtonListView::updateActiveButtons(QListWidgetItem *item) {
    CaptureButton::ButtonType bType = m_buttonTypeByName[item->text()];
    if (item->checkState() == Qt::Checked) {
        m_listButtons.append(bType);
        std::sort(m_listButtons.begin(), m_listButtons.end());
    } else {
        m_listButtons.remove(m_listButtons.indexOf(bType));
    }
    ConfigHandler().setButtons(m_listButtons);
}

void ButtonListView::reverseItemCheck(QListWidgetItem *item){
    if (item->checkState() == Qt::Checked) {
        item->setCheckState(Qt::Unchecked);
    } else {
        item->setCheckState(Qt::Checked);
    }
    updateActiveButtons(item);
}

void ButtonListView::selectAll() {
    ConfigHandler().setAllTheButtons();
    for(int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void ButtonListView::updateComponents() {
    m_listButtons = ConfigHandler().getButtons();
    auto listTypes = CaptureButton::getIterableButtonTypes();
    for(int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        auto elem = static_cast<CaptureButton::ButtonType>(listTypes.at(i));
        if (m_listButtons.contains(elem)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
}
