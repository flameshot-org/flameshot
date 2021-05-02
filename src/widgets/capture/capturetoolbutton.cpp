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

CaptureToolButton::CaptureToolButton(const ButtonType t, QWidget* parent)
  : CaptureButton(parent)
  , m_buttonType(t)
  , m_tool(nullptr)
  , m_emergeAnimation(nullptr)
{
    initButton();
    if (t == TYPE_SELECTIONINDICATOR) {
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

    setToolTip(m_tool->description());

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

QVector<CaptureToolButton::ButtonType>
CaptureToolButton::getIterableButtonTypes()
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
    if (e->button() == Qt::LeftButton) {
        emit pressedButton(this);
        emit pressed();
    }
}

void CaptureToolButton::animatedShow()
{
    if (!isVisible()) {
        show();
        m_emergeAnimation->start();
        connect(
          m_emergeAnimation, &QPropertyAnimation::finished, this, []() {});
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

QColor CaptureToolButton::m_mainColor = ConfigHandler().uiMainColorValue();

static std::map<CaptureToolButton::ButtonType, int> buttonTypeOrder
{
    { CaptureToolButton::TYPE_PENCIL, 0 },
      { CaptureToolButton::TYPE_DRAWER, 1 },
      { CaptureToolButton::TYPE_ARROW, 2 },
      { CaptureToolButton::TYPE_SELECTION, 3 },
      { CaptureToolButton::TYPE_RECTANGLE, 4 },
      { CaptureToolButton::TYPE_CIRCLE, 5 },
      { CaptureToolButton::TYPE_MARKER, 6 },
      { CaptureToolButton::TYPE_TEXT, 7 },
      { CaptureToolButton::TYPE_PIXELATE, 8 },
      { CaptureToolButton::TYPE_CIRCLECOUNT, 9 },
      { CaptureToolButton::TYPE_SELECTIONINDICATOR, 10 },
      { CaptureToolButton::TYPE_MOVESELECTION, 11 },
      { CaptureToolButton::TYPE_UNDO, 12 },
      { CaptureToolButton::TYPE_REDO, 13 },
      { CaptureToolButton::TYPE_COPY, 14 },
      { CaptureToolButton::TYPE_SAVE, 15 },
      { CaptureToolButton::TYPE_IMAGEUPLOADER, 16 },
#if not defined(Q_OS_MACOS)
      { CaptureToolButton::TYPE_OPEN_APP, 17 },
      { CaptureToolButton::TYPE_EXIT, 18 }, { CaptureToolButton::TYPE_PIN, 19 },
#else
      { CaptureToolButton::TYPE_EXIT, 17 }, { CaptureToolButton::TYPE_PIN, 18 },
#endif

      { CaptureToolButton::TYPE_SIZEINCREASE, 20 },
      { CaptureToolButton::TYPE_SIZEDECREASE, 21 },
};

int CaptureToolButton::getPriorityByButton(CaptureToolButton::ButtonType b)
{
    auto it = buttonTypeOrder.find(b);
    return it == buttonTypeOrder.cend() ? (int)buttonTypeOrder.size()
                                        : it->second;
}

QVector<CaptureToolButton::ButtonType>
  CaptureToolButton::iterableButtonTypes = {
      CaptureToolButton::TYPE_PENCIL,
      CaptureToolButton::TYPE_DRAWER,
      CaptureToolButton::TYPE_ARROW,
      CaptureToolButton::TYPE_SELECTION,
      CaptureToolButton::TYPE_RECTANGLE,
      CaptureToolButton::TYPE_CIRCLE,
      CaptureToolButton::TYPE_MARKER,
      CaptureToolButton::TYPE_TEXT,
      CaptureToolButton::TYPE_PIXELATE,
      CaptureToolButton::TYPE_SELECTIONINDICATOR,
      CaptureToolButton::TYPE_MOVESELECTION,
      CaptureToolButton::TYPE_UNDO,
      CaptureToolButton::TYPE_REDO,
      CaptureToolButton::TYPE_COPY,
      CaptureToolButton::TYPE_SAVE,
      CaptureToolButton::TYPE_EXIT,
      CaptureToolButton::TYPE_IMAGEUPLOADER,
#if not defined(Q_OS_MACOS)
      CaptureToolButton::TYPE_OPEN_APP,
#endif
      CaptureToolButton::TYPE_PIN,
      CaptureToolButton::TYPE_CIRCLECOUNT,
      CaptureToolButton::TYPE_SIZEINCREASE,
      CaptureToolButton::TYPE_SIZEDECREASE,
  };
