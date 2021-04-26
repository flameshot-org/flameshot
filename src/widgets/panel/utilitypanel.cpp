// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "utilitypanel.h"
#include "capturewidget.h"
#include <QHBoxLayout>
#include <QListWidget>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>

UtilityPanel::UtilityPanel(CaptureWidget* captureWidget)
  : QWidget(captureWidget)
  , m_captureWidget(captureWidget)
  , m_internalPanel(nullptr)
  , m_upLayout(nullptr)
  , m_bottomLayout(nullptr)
  , m_layout(nullptr)
  , m_showAnimation(nullptr)
  , m_hideAnimation(nullptr)
  , m_layersLayout(nullptr)
  , m_captureTools(nullptr)
  , m_buttonDelete(nullptr)
{
    initInternalPanel();
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setCursor(Qt::ArrowCursor);

    m_showAnimation = new QPropertyAnimation(m_internalPanel, "geometry", this);
    m_showAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_showAnimation->setDuration(300);

    m_hideAnimation = new QPropertyAnimation(m_internalPanel, "geometry", this);
    m_hideAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_hideAnimation->setDuration(300);

    connect(m_hideAnimation,
            &QPropertyAnimation::finished,
            m_internalPanel,
            &QWidget::hide);

#if (defined(Q_OS_WIN) || defined(Q_OS_MACOS))
    move(0, 0);
#endif
    hide();
}

QWidget* UtilityPanel::toolWidget() const
{
    return m_toolWidget;
}

void UtilityPanel::setToolWidget(QWidget* w)
{
    if (m_toolWidget) {
        m_toolWidget->close();
        delete m_toolWidget;
        m_toolWidget = nullptr;
    }
    if (w) {
        m_toolWidget = w;
        m_toolWidget->setSizePolicy(QSizePolicy::Ignored,
                                    QSizePolicy::Preferred);
        m_upLayout->addWidget(w);
    }
}

void UtilityPanel::clearToolWidget()
{
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
    }
}

void UtilityPanel::pushWidget(QWidget* w)
{
    m_layout->insertWidget(m_layout->count() - 1, w);
}

void UtilityPanel::show()
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_showAnimation->setStartValue(QRect(-width(), 0, 0, height()));
    m_showAnimation->setEndValue(QRect(0, 0, width(), height()));
    m_internalPanel->show();
    m_showAnimation->start();
#if (defined(Q_OS_WIN) || defined(Q_OS_MACOS))
    move(0, 0);
#endif
    QWidget::show();
}

void UtilityPanel::hide()
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_hideAnimation->setStartValue(QRect(0, 0, width(), height()));
    m_hideAnimation->setEndValue(QRect(-width(), 0, 0, height()));
    m_hideAnimation->start();
    m_internalPanel->hide();
    QWidget::hide();
}

void UtilityPanel::toggle()
{
    if (m_internalPanel->isHidden()) {
        show();
    } else {
        hide();
    }
}

void UtilityPanel::initInternalPanel()
{
    m_internalPanel = new QScrollArea(this);
    m_internalPanel->setAttribute(Qt::WA_NoMousePropagation);
    QWidget* widget = new QWidget();
    m_internalPanel->setWidget(widget);
    m_internalPanel->setWidgetResizable(true);

    m_layout = new QVBoxLayout();
    m_upLayout = new QVBoxLayout();
    m_bottomLayout = new QVBoxLayout();
    m_layersLayout = new QVBoxLayout();
    m_layout->addLayout(m_upLayout);
    m_layout->addLayout(m_bottomLayout);
    m_bottomLayout->addLayout(m_layersLayout);
    widget->setLayout(m_layout);

    QColor bgColor = palette().window().color();
    bgColor.setAlphaF(0.0);
    m_internalPanel->setStyleSheet(
      QStringLiteral("QScrollArea {background-color: %1}").arg(bgColor.name()));
    m_internalPanel->hide();

    m_captureTools = new QListWidget(this);
    connect(m_captureTools,
            SIGNAL(currentRowChanged(int)),
            this,
            SLOT(slotCaptureToolsCurrentRowChanged(int)));

    QHBoxLayout* layersButtons = new QHBoxLayout();
    m_layersLayout->addLayout(layersButtons);
    m_layersLayout->addWidget(m_captureTools);

    m_buttonDelete = new QPushButton(this);
    m_buttonDelete->setIcon(QIcon(":/img/material/black/delete.svg"));
    m_buttonDelete->setDisabled(true);
    layersButtons->addWidget(m_buttonDelete);
    layersButtons->addStretch();

    connect(m_buttonDelete,
            SIGNAL(clicked(bool)),
            this,
            SLOT(slotButtonDelete(bool)));

    // Bottom
    QPushButton* closeButton = new QPushButton(this);
    closeButton->setText(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &UtilityPanel::toggle);
    m_bottomLayout->addWidget(closeButton);
}

void UtilityPanel::fillCaptureTools(
  QList<QPointer<CaptureTool>> captureToolObjects)
{
    int currentSelection = m_captureTools->currentRow();
    m_captureTools->clear();
    m_captureTools->addItem(tr("<Empty>"));

    for (auto toolItem : captureToolObjects) {
        QListWidgetItem* item = new QListWidgetItem(
          toolItem->icon(QColor(Qt::white), false), toolItem->info());
        m_captureTools->addItem(item);
    }
    if (currentSelection >= 0 && currentSelection < m_captureTools->count()) {
        m_captureTools->setCurrentRow(currentSelection);
    }
}

void UtilityPanel::setActiveLayer(int index)
{
    index++;
    if (index >= 0 && index < m_captureTools->count()) {
        m_captureTools->setCurrentRow(index);
    }
}

int UtilityPanel::activeLayerIndex()
{
    return m_captureTools->currentRow() >= 0 ? m_captureTools->currentRow() - 1
                                             : -1;
}

void UtilityPanel::slotCaptureToolsCurrentRowChanged(int currentRow)
{
    if (currentRow > 0) {
        m_buttonDelete->setDisabled(false);
    } else {
        m_buttonDelete->setDisabled(true);
    }
    emit layerChanged(currentRow);
}

void UtilityPanel::slotButtonDelete(bool clicked)
{
    Q_UNUSED(clicked)
    int currentRow = m_captureTools->currentRow();
    if (currentRow > 0) {
        m_captureWidget->removeToolObject(currentRow);
        if (currentRow >= m_captureTools->count()) {
            currentRow = m_captureTools->count() - 1;
        }
    } else {
        currentRow = 0;
    }
    m_captureTools->setCurrentRow(currentRow);
}
