// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "buttonlistview.h"
#include "tools/toolregistry.h"
#include "utils/confighandler.h"

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
    ToolRegistry registry;

    for (const ToolDescriptor& descriptor : registry.tools()) {
        CaptureTool* tool = ToolRegistry::createTool(descriptor);
        if (!tool) {
            continue;
        }

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
        m_buttonItem->setData(Qt::UserRole, descriptor.id);
        m_buttonItem->setData(Qt::UserRole + 1, descriptor.isExternal());
        m_buttonItem->setData(Qt::UserRole + 2,
                              static_cast<int>(descriptor.legacyType));
        m_buttonItem->setData(Qt::UserRole + 3, descriptor.toolbarVisible);
        delete tool;
    }
}

void ButtonListView::updateActiveButtons(QListWidgetItem* item)
{
    const QString toolId = item->data(Qt::UserRole).toString();
    const bool external = item->data(Qt::UserRole + 1).toBool();
    if (external) {
        ConfigHandler().setPluginEnabled(toolId,
                                         item->checkState() == Qt::Checked);
        return;
    }

    const auto bType =
      static_cast<CaptureTool::Type>(item->data(Qt::UserRole + 2).toInt());
    if (item->checkState() == Qt::Checked) {
        // Refactored to avoid external sort: insert into the correct position
        using bt = CaptureTool::Type;
        auto it = std::lower_bound(
          m_listButtons.begin(), m_listButtons.end(), bType, [](bt a, bt b) {
              return CaptureToolButton::getPriorityByButton(a) <
                     CaptureToolButton::getPriorityByButton(b);
          });
        m_listButtons.insert(it, bType);
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
        if (item->data(Qt::UserRole + 1).toBool()) {
            ConfigHandler().setPluginEnabled(
              item->data(Qt::UserRole).toString(), true);
        }
    }
}

void ButtonListView::reload()
{
    clear();
    initButtonList();
    updateComponents();
}

QString ButtonListView::selectedExternalPluginId() const
{
    const QListWidgetItem* selected = currentItem();
    if (!selected || !selected->data(Qt::UserRole + 1).toBool()) {
        return {};
    }
    return selected->data(Qt::UserRole).toString();
}

void ButtonListView::updateComponents()
{
    m_listButtons = ConfigHandler().buttons();
    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        bool enabled;
        if (item->data(Qt::UserRole + 1).toBool()) {
            enabled = ConfigHandler().pluginEnabled(
              item->data(Qt::UserRole).toString(),
              item->data(Qt::UserRole + 3).toBool());
        } else {
            const auto type = static_cast<CaptureTool::Type>(
              item->data(Qt::UserRole + 2).toInt());
            enabled = m_listButtons.contains(type);
        }
        if (enabled) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
}
