// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturebutton.h"
#include "src/utils/colorutils.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QGraphicsDropShadowEffect>

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

    auto* dsEffect = new QGraphicsDropShadowEffect(this);
    dsEffect->setBlurRadius(5);
    dsEffect->setOffset(0);
    dsEffect->setColor(QColor(Qt::black));

    setGraphicsEffect(dsEffect);
}

QString CaptureButton::globalStyleSheet()
{
    return CaptureButton(nullptr).styleSheet();
}

QString CaptureButton::styleSheet() const
{
    QString baseSheet = "CaptureButton { border: none;"
                        "padding: 3px 8px;"
                        "background-color: %1; color: %4 }"
                        "CaptureToolButton { border-radius: %3;"
                        "padding: 0; }"
                        "CaptureButton:hover { background-color: %2; }"
                        "CaptureButton:pressed:!hover { "
                        "background-color: %1; }";
    // define color when mouse is hovering
    QColor contrast = ColorUtils::contrastColor(m_mainColor);
    // foreground color
    QColor color = ColorUtils::colorIsDark(m_mainColor) ? Qt::white : Qt::black;

    return baseSheet.arg(m_mainColor.name())
      .arg(contrast.name())
      .arg(GlobalValues::buttonBaseSize() / 2)
      .arg(color.name());
}

void CaptureButton::setColor(const QColor& c)
{
    m_mainColor = c;
    setStyleSheet(styleSheet());
}

QColor CaptureButton::m_mainColor;
