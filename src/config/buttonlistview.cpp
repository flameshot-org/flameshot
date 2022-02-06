// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "buttonlistview.h"
#include "src/tools/toolfactory.h"
#include "src/utils/confighandler.h"
#include <QListWidgetItem>
#include <algorithm>

ButtonListView::ButtonListView(QWidget* parent)
  : QListWidget(parent)
{
    setMouseTracking(true);
    setFlow(QListWidget::TopToBottom);
    initButtonList();
    updateComponents();
    connect(
      this, &QListWidget::itemClicked, this, &ButtonListView::reverseItemCheck);
}

void ButtonListView::initButtonList()
{
    ToolFactory factory;
    auto listTypes = CaptureToolButton::getIterableButtonTypes();

    for (const CaptureTool::Type t : listTypes) {
        CaptureTool* tool = factory.CreateTool(t);

        // add element to the local map
        m_buttonTypeByName.insert(tool->name(), t);

        // init the menu option
        auto* m_buttonItem = new QListWidgetItem(this);

        // when the background is lighter than gray, it uses the white icons
        QColor bgColor = this->palette().color(QWidget::backgroundRole());
        m_buttonItem->setIcon(tool->icon(bgColor, false));

        m_buttonItem->setFlags(Qt::ItemIsUserCheckable);
        QColor foregroundColor =
          this->palette().color(QWidget::foregroundRole());
        m_buttonItem->setForeground(foregroundColor);

        m_buttonItem->setText(tool->name());
        m_buttonItem->setToolTip(tool->description());
        tool->deleteLater();
    }
}

void ButtonListView::updateActiveButtons(QListWidgetItem* item)
{
    CaptureTool::Type bType = m_buttonTypeByName[item->text()];
    if (item->checkState() == Qt::Checked) {
        m_listButtons.append(bType);
        // TODO refactor so we don't need external sorts
        using bt = CaptureTool::Type;
        std::sort(m_listButtons.begin(), m_listButtons.end(), [](bt a, bt b) {
            return CaptureToolButton::getPriorityByButton(a) <
                   CaptureToolButton::getPriorityByButton(b);
        });
    } else {
        m_listButtons.removeOne(bType);
    }
    ConfigHandler().setButtons(m_listButtons);
}

void ButtonListView::reverseItemCheck(QListWidgetItem* item)
{
    if (item->checkState() == Qt::Checked) {
        item->setCheckState(Qt::Unchecked);
    } else {
        item->setCheckState(Qt::Checked);
    }
    updateActiveButtons(item);
}

void ButtonListView::selectAll()
{
    ConfigHandler().setAllTheButtons();
    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void ButtonListView::updateComponents()
{
    m_listButtons = ConfigHandler().buttons();
    auto listTypes = CaptureToolButton::getIterableButtonTypes();
    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        auto elem = static_cast<CaptureTool::Type>(listTypes.at(i));
        if (m_listButtons.contains(elem)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
}
