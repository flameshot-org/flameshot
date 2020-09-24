// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

    auto dsEffect = new QGraphicsDropShadowEffect(this);
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

QColor CaptureButton::m_mainColor = ConfigHandler().uiMainColorValue();
