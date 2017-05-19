#include "buttonlistview.h"
#include "capture/button.h"
#include <QListWidgetItem>
#include <QListWidgetItem>
#include <QSettings>

ButtonListView::ButtonListView(QWidget *parent) : QListWidget(parent) {
    setMouseTracking(true);
    //setFocusPolicy(Qt::NoFocus);

    QSettings settings;
    m_listButtons = settings.value("buttons").value<QList<int> >();
    initButtonList();

    connect(this, &QListWidget::itemChanged, this,
            &ButtonListView::updateActiveButtons);
    connect(this, &QListWidget::itemClicked, this,
            &ButtonListView::reverseItemCheck);
}

void ButtonListView::initButtonList() {
    for (int i = 0; i != static_cast<int>(Button::Type::last); ++i) {
        auto t = static_cast<Button::Type>(i);
        QListWidgetItem *buttonItem = new QListWidgetItem(this);

        bool iconsAreWhite = false;
        QString bgColor = this->palette().color(QWidget::backgroundRole()).name();
        // when the background is lighter than gray, it uses the white icons
        if (bgColor < QColor(Qt::gray).name()) {
            iconsAreWhite = true;
        }
        buttonItem->setIcon(Button::getIcon(t, iconsAreWhite));
        buttonItem->setFlags(Qt::ItemIsUserCheckable);

        buttonItem->setText(Button::getTypeName(t));
        buttonItem->setToolTip(Button::getTypeTooltip(t));
        if (m_listButtons.contains(i)) {
            buttonItem->setCheckState(Qt::Checked);
        } else {
            buttonItem->setCheckState(Qt::Unchecked);
        }
    }
}

void ButtonListView::updateActiveButtons(QListWidgetItem *item) {
    int buttonIndex = static_cast<int>(Button::getTypeByName(item->text()));
    if (item->checkState() == Qt::Checked) {
        m_listButtons.append(buttonIndex);
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
