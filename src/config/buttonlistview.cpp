// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "buttonlistview.h"
#include "src/tools/toolfactory.h"
#include "src/utils/confighandler.h"
#include "src/widgets/checkablestar.h"
#include <QListWidgetItem>
#include <QHBoxLayout>
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

        // create the widget to contain the star
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);
        CheckableStar* checkableStar = new CheckableStar();

        layout->addWidget(checkableStar);

        m_buttonItem->setSizeHint(widget->sizeHint());

        this->setItemWidget(m_buttonItem, widget);

        // Connect the starredChanged signal to update favorite buttons
        connect(checkableStar, &CheckableStar::starredChanged, [this, t](bool starred) {
            if (starred) {
                m_favoriteButtons.append(t);
            } else {
                m_favoriteButtons.removeOne(t);
            }
            ConfigHandler().setFavoriteButtons(m_favoriteButtons);
        });

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
    m_favoriteButtons = ConfigHandler().favoriteButtons();
    auto listTypes = CaptureToolButton::getIterableButtonTypes();
    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        auto elem = static_cast<CaptureTool::Type>(listTypes.at(i));
        if (m_listButtons.contains(elem)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }

        QWidget* widget = this->itemWidget(item);
        CheckableStar* checkableStar = widget->findChild<CheckableStar*>();
        checkableStar->setStarred(m_favoriteButtons.contains(elem));
    }
}
