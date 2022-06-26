//
// Created by yuriypuchkov on 09.12.2020.
//

#include "updatenotificationwidget.h"
#include "src/utils/confighandler.h"
#include <QDesktopServices>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <utility>

UpdateNotificationWidget::UpdateNotificationWidget(
  QWidget* parent,
  const QString& appLatestVersion,
  QString appLatestUrl)
  : QWidget(parent)
  , m_appLatestVersion(appLatestVersion)
  , m_appLatestUrl(std::move(appLatestUrl))
  , m_layout(nullptr)
{
    setMinimumSize(400, 100);
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
    setAppLatestVersion(appLatestVersion);
}

void UpdateNotificationWidget::show()
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_showAnimation->setStartValue(QRect(0, -height(), width(), height()));
    m_showAnimation->setEndValue(QRect(0, 0, width(), height()));
    m_internalPanel->show();
    m_showAnimation->start();
    QWidget::show();
}

void UpdateNotificationWidget::hide()
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_hideAnimation->setStartValue(QRect(0, 0, width(), height()));
    m_hideAnimation->setEndValue(QRect(0, -height(), 0, height()));
    m_hideAnimation->start();
    m_internalPanel->hide();
    QWidget::hide();
}

void UpdateNotificationWidget::setAppLatestVersion(const QString& latestVersion)
{
    m_appLatestVersion = latestVersion;
    QString newVersion =
      tr("New Flameshot version %1 is available").arg(latestVersion);
    m_notification->setText(newVersion);
}

void UpdateNotificationWidget::laterButton()
{
    hide();
}

void UpdateNotificationWidget::ignoreButton()
{
    ConfigHandler().setIgnoreUpdateToVersion(m_appLatestVersion);
    hide();
}

void UpdateNotificationWidget::updateButton()
{
    QDesktopServices::openUrl(m_appLatestUrl);
    hide();
    if (parentWidget()) {
        parentWidget()->close();
    }
}

void UpdateNotificationWidget::initInternalPanel()
{
    m_internalPanel = new QScrollArea(this);
    m_internalPanel->setAttribute(Qt::WA_NoMousePropagation);
    auto* widget = new QWidget();
    m_internalPanel->setWidget(widget);
    m_internalPanel->setWidgetResizable(true);

    QColor bgColor = palette().window().color();
    bgColor.setAlphaF(0.0);
    m_internalPanel->setStyleSheet(
      QStringLiteral("QScrollArea {background-color: %1}").arg(bgColor.name()));
    m_internalPanel->hide();

    //
    m_layout = new QVBoxLayout();
    widget->setLayout(m_layout);

    // caption
    m_notification = new QLabel(m_appLatestVersion, this);
    m_layout->addWidget(m_notification);

    // buttons layout
    auto* buttonsLayout = new QHBoxLayout();
    auto* bottonsSpacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    buttonsLayout->addSpacerItem(bottonsSpacer);
    m_layout->addLayout(buttonsLayout);

    // ignore
    auto* ignoreBtn = new QPushButton(tr("Ignore"), this);
    buttonsLayout->addWidget(ignoreBtn);
    connect(ignoreBtn,
            &QPushButton::clicked,
            this,
            &UpdateNotificationWidget::ignoreButton);

    // later
    auto* laterBtn = new QPushButton(tr("Later"), this);
    buttonsLayout->addWidget(laterBtn);
    connect(laterBtn,
            &QPushButton::clicked,
            this,
            &UpdateNotificationWidget::laterButton);

    // update
    auto* updateBtn = new QPushButton(tr("Update"), this);
    buttonsLayout->addWidget(updateBtn);
    connect(updateBtn,
            &QPushButton::clicked,
            this,
            &UpdateNotificationWidget::updateButton);
}
