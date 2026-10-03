// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Jeremy Borgman & Contributors

#include "monitorpreview.h"
#include "utils/colorutils.h"
#include "utils/confighandler.h"

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
  , m_selected(false)
  , m_mouseHovered(false)
  , m_imageLabel(nullptr)
  , m_keyLabel(nullptr)
  , m_textLabel(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setPixmap(thumbnail);
    m_imageLabel->setScaledContents(false);

    if (m_monitorIndex < 9) {
        m_keyLabel =
          new QLabel(QString::number(m_monitorIndex + 1), m_imageLabel);
        m_keyLabel->setAlignment(Qt::AlignCenter);
        m_keyLabel->setFixedSize(28, 28);
        m_keyLabel->move(8, 8);
        m_keyLabel->raise();
        m_keyLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    const QString labelText =
      m_monitorIndex < 9 ? tr("Monitor %1: %2\nClick or press %1 to select")
                             .arg(m_monitorIndex + 1)
                             .arg(screen->name())
                         : tr("Monitor %1: %2\nClick to select")
                             .arg(m_monitorIndex + 1)
                             .arg(screen->name());
    m_textLabel = new QLabel(labelText, this);
    m_textLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_imageLabel);
    layout->addWidget(m_textLabel);

    m_uiColor = ConfigHandler().uiColor();
    m_contrastColor = ColorUtils::contrastColor(m_uiColor);

    updateStyle();
}

void MonitorPreview::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    emit monitorSelected(m_monitorIndex);
}

void MonitorPreview::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    m_mouseHovered = true;
    updateStyle();
}

void MonitorPreview::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_mouseHovered = false;
    updateStyle();
}

void MonitorPreview::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }

    m_selected = selected;
    updateStyle();
}

void MonitorPreview::updateStyle()
{
    const bool highlighted = m_selected || m_mouseHovered;
    const QColor backgroundColor = highlighted ? m_contrastColor : m_uiColor;
    const int textAlpha = highlighted ? 220 : 200;
    const int borderAlpha = highlighted ? 255 : 0;

    QString textStyle =
      QString("QLabel { color: white; background-color: rgba(%1, %2, %3, %4); "
              "padding: 5px; font-size: 12pt; border-radius: 3px; }")
        .arg(backgroundColor.red())
        .arg(backgroundColor.green())
        .arg(backgroundColor.blue())
        .arg(textAlpha);
    m_textLabel->setStyleSheet(textStyle);

    QString imageStyle =
      QString("QLabel { background-color: black; border: 2px solid "
              "rgba(%1, %2, %3, %4); border-radius: 8px; }")
        .arg(backgroundColor.red())
        .arg(backgroundColor.green())
        .arg(backgroundColor.blue())
        .arg(borderAlpha);
    m_imageLabel->setStyleSheet(imageStyle);

    if (m_keyLabel) {
        QString keyStyle = QString("QLabel { color: white; font-weight: bold; "
                                   "background-color: rgba(%1, %2, %3, 230); "
                                   "border-radius: 14px; }")
                             .arg(backgroundColor.red())
                             .arg(backgroundColor.green())
                             .arg(backgroundColor.blue());
        m_keyLabel->setStyleSheet(keyStyle);
    }
}
