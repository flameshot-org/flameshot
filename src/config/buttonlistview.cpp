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
#include "src/capture/capturebutton.h"
#include <QListWidgetItem>
#include <QListWidgetItem>
#include <QSettings>
#include <algorithm>

ButtonListView::ButtonListView(QWidget *parent) : QListWidget(parent) {
    setMouseTracking(true);

    QSettings settings;
    m_listButtons = settings.value("buttons").value<QList<int> >();

    initButtonList();

    connect(this, &QListWidget::itemChanged, this,
            &ButtonListView::updateActiveButtons);
    connect(this, &QListWidget::itemClicked, this,
            &ButtonListView::reverseItemCheck);
}

void ButtonListView::initButtonList() {
    for (int i = 0; i != static_cast<int>(CaptureButton::Type::last); ++i) {
        auto t = static_cast<CaptureButton::Type>(i);
        QListWidgetItem *buttonItem = new QListWidgetItem(this);

        bool iconsAreWhite = false;
        QColor bgColor = this->palette().color(QWidget::backgroundRole());
        // when the background is lighter than gray, it uses the white icons
        if (bgColor.valueF() < 0.6) {
            iconsAreWhite = true;
        }
        buttonItem->setIcon(CaptureButton::getIcon(t, iconsAreWhite));
        buttonItem->setFlags(Qt::ItemIsUserCheckable);
        QColor foregroundColor = this->palette().color(QWidget::foregroundRole());
        buttonItem->setTextColor(foregroundColor);

        buttonItem->setText(CaptureButton::getTypeName(t));
        buttonItem->setToolTip(CaptureButton::getTypeTooltip(t));
        if (m_listButtons.contains(i)) {
            buttonItem->setCheckState(Qt::Checked);
        } else {
            buttonItem->setCheckState(Qt::Unchecked);
        }
    }
}

void ButtonListView::updateActiveButtons(QListWidgetItem *item) {
    int buttonIndex = static_cast<int>(CaptureButton::getTypeByName(item->text()));

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
