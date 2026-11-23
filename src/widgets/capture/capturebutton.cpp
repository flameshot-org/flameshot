// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturebutton.h"
#include "utils/colorutils.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"

CaptureButton::CaptureButton(QWidget* parent)
  : QPushButton(parent)
{
    init();
}

CaptureButton::CaptureButton(const QString& text, QWidget* parent)
  : QPushButton(text, parent)
{
    init();
}

CaptureButton::CaptureButton(const QIcon& icon,
                             const QString& text,
                             QWidget* parent)
  : QPushButton(icon, text, parent)
{
    init();
}

void CaptureButton::init()
{
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
}

QString CaptureButton::globalStyleSheet()
{
    return styleSheetForColor(ConfigHandler().uiColor());
}

QString CaptureButton::styleSheetForColor(const QColor& c)
{
    QString baseSheet = "CaptureButton { border: 1px solid rgba(0, 0, 0, 90);"
                        "padding: 3px 8px;"
                        "background-color: %1; color: %4 }"
                        "CaptureToolButton { border-radius: %3;"
                        "padding: 0; }"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = ColorUtils::contrastColor(c);
    // foreground color
    QColor color = ColorUtils::colorIsDark(c) ? Qt::white : Qt::black;

    return baseSheet.arg(c.name(),
                         contrast.name(),
                         QString::number(GlobalValues::buttonBaseSize() / 2),
                         color.name());
}

QString CaptureButton::styleSheet() const
{
    return styleSheetForColor(m_color);
}

void CaptureButton::setColor(const QColor& c)
{
    if (m_color == c && !QPushButton::styleSheet().isEmpty()) {
        return;
    }
    m_color = c;
    setStyleSheet(styleSheetForColor(c));
}

void CaptureButton::setInheritedColor(const QColor& c)
{
    m_color = c;
    if (!QPushButton::styleSheet().isEmpty()) {
        setStyleSheet(QString());
    }
}
