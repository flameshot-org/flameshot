// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturetoolbutton.h"
#include "tools/capturetool.h"
#include "tools/toolfactory.h"
#include "tools/toolregistry.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"

#include <QApplication>
#include <QIcon>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QToolTip>

// Button represents a single button of the capture widget, it can enable
// multiple functionality.

CaptureToolButton::CaptureToolButton(const CaptureTool::Type t, QWidget* parent)
  : CaptureButton(parent)
  , m_buttonType(t)
  , m_toolId(ToolRegistry::builtInId(t))
  , m_external(false)
  , m_tool(nullptr)
  , m_emergeAnimation(nullptr)
{
    initButton();
    updateIcon();
}

CaptureToolButton::CaptureToolButton(const ToolDescriptor& descriptor,
                                     QWidget* parent)
  : CaptureButton(parent)
  , m_buttonType(descriptor.legacyType)
  , m_toolId(descriptor.id)
  , m_shortcut(descriptor.shortcut)
  , m_external(descriptor.isExternal())
  , m_tool(ToolRegistry::createTool(descriptor, this))
  , m_emergeAnimation(nullptr)
{
    initButton();
    updateIcon();
}

CaptureToolButton::~CaptureToolButton()
{
    if (m_tool) {
        delete m_tool;
        m_tool = nullptr;
    }
    if (m_emergeAnimation) {
        delete m_emergeAnimation;
        m_emergeAnimation = nullptr;
    }
}

void CaptureToolButton::initButton()
{
    if (!m_tool) {
        m_tool = ToolFactory().CreateTool(m_buttonType, this);
    }

    resize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize());
    setMask(QRegion(QRect(-1,
                          -1,
                          GlobalValues::buttonBaseSize() + 2,
                          GlobalValues::buttonBaseSize() + 2),
                    QRegion::Ellipse));

    // Set a tooltip showing a shortcut in parentheses (if there is a shortcut)
    QString tooltip = m_tool->description();
    QString shortcut = m_shortcut;
    if (!m_external && shortcut.isNull()) {
        shortcut = ConfigHandler().shortcut(
          QVariant::fromValue(m_buttonType).toString());
    }
    if (m_buttonType == CaptureTool::TYPE_COPY &&
        ConfigHandler().copyOnDoubleClick()) {
        tooltip += QStringLiteral(" (%1Left Double-Click)")
                     .arg(shortcut.isEmpty() ? QString() : shortcut + " or ");
    } else if (!shortcut.isEmpty()) {
        tooltip += QStringLiteral(" (%1)").arg(shortcut);
    }
    tooltip.replace("Return", "Enter");
    setToolTip(tooltip);

    m_emergeAnimation = new QPropertyAnimation(this, "size", this);
    m_emergeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_emergeAnimation->setDuration(80);
    m_emergeAnimation->setStartValue(QSize(0, 0));
    m_emergeAnimation->setEndValue(
      QSize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize()));
}

void CaptureToolButton::updateIcon()
{
    setIcon(icon());
    setIconSize(size() * 0.6);
}

const QList<CaptureTool::Type>& CaptureToolButton::getIterableButtonTypes()
{
    return ToolRegistry::builtInTypes();
}

// get icon returns the icon for the type of button
QIcon CaptureToolButton::icon() const
{
    return m_tool->icon(m_mainColor, true);
}

void CaptureToolButton::mousePressEvent(QMouseEvent* e)
{
    const bool closesWindowOnPress = m_tool && m_tool->closeOnButtonPressed();
    if (QGuiApplication::platformName() != QLatin1String("wayland") ||
        !closesWindowOnPress) {
        activateWindow();
    }
    if (e->button() == Qt::LeftButton) {
        emit pressedButtonLeftClick(this);
        emit pressed();
    } else if (e->button() == Qt::RightButton) {
        emit pressedButtonRightClick(this);
        emit pressed();
    }
}

void CaptureToolButton::animatedShow()
{
    if (!isVisible()) {
        show();
        m_emergeAnimation->start();
        connect(m_emergeAnimation,
                &QPropertyAnimation::finished,
                this,
                [this]() { updateIcon(); });
    }
}

CaptureTool* CaptureToolButton::tool() const
{
    return m_tool;
}

QString CaptureToolButton::toolId() const
{
    return m_toolId;
}

void CaptureToolButton::setColor(const QColor& c)
{
    m_mainColor = c;
    CaptureButton::setColor(c);
    updateIcon();
}

QColor CaptureToolButton::m_mainColor;

int CaptureToolButton::getPriorityByButton(CaptureTool::Type b)
{
    return ToolRegistry::builtInPriority(b);
}
