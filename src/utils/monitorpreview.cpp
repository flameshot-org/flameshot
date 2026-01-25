// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Jeremy Borgman & Contributors

#include "monitorpreview.h"
#include "src/utils/colorutils.h"
#include "src/utils/confighandler.h"
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QVBoxLayout>

MonitorPreview::MonitorPreview(int monitorIndex,
                               QScreen* screen,
                               const QPixmap& thumbnail,
                               QWidget* parent)
  : QWidget(parent)
  , m_monitorIndex(monitorIndex)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    QLabel* imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setPixmap(thumbnail);
    imageLabel->setStyleSheet(
      "QLabel { background-color: black; border-radius: 8px; }");
    imageLabel->setScaledContents(false);

    m_textLabel = new QLabel(tr("Monitor %1: %2\nClick to select")
                               .arg(m_monitorIndex + 1)
                               .arg(screen->name()),
                             this);
    m_textLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(imageLabel);
    layout->addWidget(m_textLabel);

    m_uiColor = ConfigHandler().uiColor();
    m_contrastColor = ColorUtils::contrastColor(m_uiColor);

    // Apply initial themed background to text label only
    QString normalStyle =
      QString("QLabel { color: white; background-color: rgba(%1, %2, %3, 200); "
              "padding: 5px; font-size: 12pt; border-radius: 3px; }")
        .arg(m_uiColor.red())
        .arg(m_uiColor.green())
        .arg(m_uiColor.blue());
    m_textLabel->setStyleSheet(normalStyle);
}

void MonitorPreview::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    emit monitorSelected(m_monitorIndex);
}

void MonitorPreview::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    QColor hoverBg = m_contrastColor;
    QString hoverStyle =
      QString("QLabel { color: white; background-color: rgba(%1, %2, %3, 220); "
              "padding: 5px; font-size: 12pt; border-radius: 3px; }")
        .arg(hoverBg.red())
        .arg(hoverBg.green())
        .arg(hoverBg.blue());
    m_textLabel->setStyleSheet(hoverStyle);
}

void MonitorPreview::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    QString normalStyle =
      QString("QLabel { color: white; background-color: rgba(%1, %2, %3, 200); "
              "padding: 5px; font-size: 12pt; border-radius: 3px; }")
        .arg(m_uiColor.red())
        .arg(m_uiColor.green())
        .arg(m_uiColor.blue());
    m_textLabel->setStyleSheet(normalStyle);
}
