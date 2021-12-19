// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturetoolbutton.h"
#include "src/tools/capturetool.h"
#include "src/tools/toolfactory.h"
#include "src/utils/colorutils.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
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
  , m_tool(nullptr)
  , m_emergeAnimation(nullptr)
{
    initButton();
    if (t == CaptureTool::TYPE_SELECTIONINDICATOR) {
        QFont f = this->font();
        setFont(QFont(f.family(), 7, QFont::Bold));
    } else {
        updateIcon();
    }
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
    if (m_tool) {
        delete m_tool;
        m_tool = nullptr;
    }
    m_tool = ToolFactory().CreateTool(m_buttonType, this);

    resize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize());
    setMask(QRegion(QRect(-1,
                          -1,
                          GlobalValues::buttonBaseSize() + 2,
                          GlobalValues::buttonBaseSize() + 2),
                    QRegion::Ellipse));

    // Set a tooltip showing a shortcut in parentheses (if there is a shortcut)
    QString tooltip = m_tool->description();
    QString shortcut =
      ConfigHandler().shortcut(QVariant::fromValue(m_buttonType).toString());
    if (m_buttonType == CaptureTool::TYPE_COPY) {
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
    return iterableButtonTypes;
}

// get icon returns the icon for the type of button
QIcon CaptureToolButton::icon() const
{
    return m_tool->icon(m_mainColor, true);
}

void CaptureToolButton::mousePressEvent(QMouseEvent* e)
{
    activateWindow();
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

void CaptureToolButton::setColor(const QColor& c)
{
    m_mainColor = c;
    CaptureButton::setColor(c);
    updateIcon();
}

QColor CaptureToolButton::m_mainColor;

static std::map<CaptureTool::Type, int> buttonTypeOrder
{
    { CaptureTool::TYPE_PENCIL, 0 }, { CaptureTool::TYPE_DRAWER, 1 },
      { CaptureTool::TYPE_ARROW, 2 }, { CaptureTool::TYPE_SELECTION, 3 },
      { CaptureTool::TYPE_RECTANGLE, 4 }, { CaptureTool::TYPE_CIRCLE, 5 },
      { CaptureTool::TYPE_MARKER, 6 }, { CaptureTool::TYPE_TEXT, 7 },
      { CaptureTool::TYPE_PIXELATE, 8 }, { CaptureTool::TYPE_INVERT, 9 },
      { CaptureTool::TYPE_CIRCLECOUNT, 10 },
      { CaptureTool::TYPE_SELECTIONINDICATOR, 11 },
      { CaptureTool::TYPE_MOVESELECTION, 12 }, { CaptureTool::TYPE_UNDO, 13 },
      { CaptureTool::TYPE_REDO, 14 }, { CaptureTool::TYPE_COPY, 15 },
      { CaptureTool::TYPE_SAVE, 16 }, { CaptureTool::TYPE_IMAGEUPLOADER, 17 },
      { CaptureTool::TYPE_ACCEPT, 18 },
#if !defined(Q_OS_MACOS)
      { CaptureTool::TYPE_OPEN_APP, 19 }, { CaptureTool::TYPE_EXIT, 20 },
      { CaptureTool::TYPE_PIN, 21 },
#else
      { CaptureTool::TYPE_EXIT, 19 }, { CaptureTool::TYPE_PIN, 20 },
#endif

      { CaptureTool::TYPE_SIZEINCREASE, 22 },
      { CaptureTool::TYPE_SIZEDECREASE, 23 },
};

int CaptureToolButton::getPriorityByButton(CaptureTool::Type b)
{
    auto it = buttonTypeOrder.find(b);
    return it == buttonTypeOrder.cend() ? (int)buttonTypeOrder.size()
                                        : it->second;
}

QList<CaptureTool::Type> CaptureToolButton::iterableButtonTypes = {
    CaptureTool::TYPE_PENCIL,        CaptureTool::TYPE_DRAWER,
    CaptureTool::TYPE_ARROW,         CaptureTool::TYPE_SELECTION,
    CaptureTool::TYPE_RECTANGLE,     CaptureTool::TYPE_CIRCLE,
    CaptureTool::TYPE_MARKER,        CaptureTool::TYPE_TEXT,
    CaptureTool::TYPE_CIRCLECOUNT,   CaptureTool::TYPE_PIXELATE,
    CaptureTool::TYPE_INVERT,        CaptureTool::TYPE_SELECTIONINDICATOR,
    CaptureTool::TYPE_MOVESELECTION, CaptureTool::TYPE_UNDO,
    CaptureTool::TYPE_REDO,          CaptureTool::TYPE_COPY,
    CaptureTool::TYPE_SAVE,          CaptureTool::TYPE_EXIT,
    CaptureTool::TYPE_IMAGEUPLOADER,
#if !defined(Q_OS_MACOS)
    CaptureTool::TYPE_OPEN_APP,
#endif
    CaptureTool::TYPE_PIN,           CaptureTool::TYPE_SIZEINCREASE,
    CaptureTool::TYPE_SIZEDECREASE,  CaptureTool::TYPE_ACCEPT,
};
