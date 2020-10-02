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

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#include "capturewidget.h"
#include "src/core/controller.h"
#include "src/tools/storage/storagemanager.h"
#include "src/tools/toolfactory.h"
#include "src/utils/colorutils.h"
#include "src/utils/screengrabber.h"
#include "src/utils/screenshotsaver.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/capture/colorpicker.h"
#include "src/widgets/capture/hovereventfilter.h"
#include "src/widgets/capture/modificationcommand.h"
#include "src/widgets/capture/notifierbox.h"
#include "src/widgets/orientablepushbutton.h"
#include "src/widgets/panel/sidepanelwidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QUndoView>
#include <draggablewidgetmaker.h>
// CaptureWidget is the main component used to capture the screen. It contains
// an area of selection with its respective buttons.

// enableSaveWIndow
CaptureWidget::CaptureWidget(const uint id,
                             const QString& savePath,
                             bool fullScreen,
                             QWidget* parent)
  : QWidget(parent)
  , m_mouseIsClicked(false)
  , m_rightClick(false)
  , m_newSelection(false)
  , m_grabbing(false)
  , m_captureDone(false)
  , m_previewEnabled(true)
  , m_adjustmentButtonPressed(false)
  , m_activeButton(nullptr)
  , m_activeTool(nullptr)
  , m_toolWidget(nullptr)
  , m_mouseOverHandle(SelectionWidget::NO_SIDE)
  , m_id(id)
{
    // Base config of the widget
    m_eventFilter = new HoverEventFilter(this);
    connect(m_eventFilter,
            &HoverEventFilter::hoverIn,
            this,
            &CaptureWidget::childEnter);
    connect(m_eventFilter,
            &HoverEventFilter::hoverOut,
            this,
            &CaptureWidget::childLeave);
    setAttribute(Qt::WA_DeleteOnClose);
    m_showInitialMsg = m_config.showHelpValue();
    m_opacity = m_config.contrastOpacityValue();
    setMouseTracking(true);
    initContext(savePath, fullScreen);
    initShortcuts();
    m_context.circleCount = 1;
#ifdef Q_OS_WIN
    // Top left of the whole set of screens
    QPoint topLeft(0, 0);
#endif
    if (fullScreen) {
        // Grab Screenshot
        bool ok = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(ok);
        if (!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
            this->close();
        }
        m_context.origScreenshot = m_context.screenshot;

#ifdef Q_OS_WIN
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                       Qt::Popup);

        for (QScreen* const screen : QGuiApplication::screens()) {
            QPoint topLeftScreen = screen->geometry().topLeft();
            if (topLeft.x() > topLeftScreen.x() ||
                topLeft.y() > topLeftScreen.y()) {
                topLeft = topLeftScreen;
            }
        }
        move(topLeft);
#else
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint |
                       Qt::FramelessWindowHint | Qt::Tool);
#endif
        resize(pixmap().size());
    }
    // Create buttons
    m_buttonHandler = new ButtonHandler(this);
    updateButtons();
    QVector<QRect> areas;
    if (m_context.fullscreen) {
        for (QScreen* const screen : QGuiApplication::screens()) {
            QRect r = screen->geometry();
            r.moveTo(r.x() / screen->devicePixelRatio(),
                     r.y() / screen->devicePixelRatio());
#ifdef Q_OS_WIN
            r.moveTo(r.topLeft() - topLeft);
#endif
            areas.append(r);
        }
    } else {
        areas.append(rect());
    }
    m_buttonHandler->updateScreenRegions(areas);
    m_buttonHandler->hide();

    initSelection();
    updateCursor();

    // Init color picker
    m_colorPicker = new ColorPicker(this);
    connect(m_colorPicker,
            &ColorPicker::colorSelected,
            this,
            &CaptureWidget::setDrawColor);
    m_colorPicker->hide();

    // Init notification widget
    m_notifierBox = new NotifierBox(this);
    m_notifierBox->hide();

    connect(&m_undoStack, &QUndoStack::indexChanged, this, [this](int) {
        this->update();
    });
    initPanel();
}

CaptureWidget::~CaptureWidget()
{
    if (m_captureDone) {
        emit captureTaken(m_id, this->pixmap());
    } else {
        emit captureFailed(m_id);
    }
    m_config.setdrawThickness(m_context.thickness);
}

// redefineButtons retrieves the buttons configured to be shown with the
// selection in the capture
void CaptureWidget::updateButtons()
{
    m_uiColor = m_config.uiMainColorValue();
    m_contrastUiColor = m_config.uiContrastColorValue();

    auto buttons = m_config.getButtons();
    QVector<CaptureToolButton*> vectorButtons;

    for (const CaptureToolButton::ButtonType& t : buttons) {
        CaptureToolButton* b = new CaptureToolButton(t, this);
        if (t == CaptureToolButton::TYPE_SELECTIONINDICATOR) {
            m_sizeIndButton = b;
        }
        b->setColor(m_uiColor);
        makeChild(b);

        switch (t) {
            case CaptureToolButton::ButtonType::TYPE_EXIT:
            case CaptureToolButton::ButtonType::TYPE_SAVE:
            case CaptureToolButton::ButtonType::TYPE_COPY:
            case CaptureToolButton::ButtonType::TYPE_UNDO:
            case CaptureToolButton::ButtonType::TYPE_REDO:
            case CaptureToolButton::ButtonType::TYPE_IMAGEUPLOADER:
                // nothing to do, just skip non-dynamic buttons with existing
                // hard coded slots
                break;
            default:
                // Set shortcuts for a tool
                QString shortcut =
                  ConfigHandler().shortcut(QVariant::fromValue(t).toString());
                if (!shortcut.isNull()) {
                    QShortcut* key =
                      new QShortcut(QKeySequence(shortcut), this);
                    CaptureWidget* captureWidget = this;
                    connect(key, &QShortcut::activated, this, [=]() {
                        emit captureWidget->setState(b);
                    });
                }
                break;
        }

        connect(
          b, &CaptureToolButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(),
                &CaptureTool::requestAction,
                this,
                &CaptureWidget::handleButtonSignal);

        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
}

QPixmap CaptureWidget::pixmap()
{
    QPixmap p;
    if (m_toolWidget && m_activeTool) {
        p = m_context.selectedScreenshotArea().copy();
        QPainter painter(&p);
        m_activeTool->process(painter, p);
    } else {
        p = m_context.selectedScreenshotArea();
    }
    return m_context.selectedScreenshotArea();
}

void CaptureWidget::deleteToolwidgetOrClose()
{
    if (m_panel->isVisible()) {
        m_panel->hide();
    } else if (m_toolWidget) {
        m_toolWidget->deleteLater();
        m_toolWidget = nullptr;
    } else {
        close();
    }
}

void CaptureWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_context.screenshot);

    if (m_activeTool && m_mouseIsClicked) {
        painter.save();
        m_activeTool->process(painter, m_context.screenshot);
        painter.restore();
    } else if (m_activeButton && m_activeButton->tool()->showMousePreview() &&
               m_previewEnabled) {
        painter.save();
        m_activeButton->tool()->paintMousePreview(painter, m_context);
        painter.restore();
    }

    QColor overlayColor(0, 0, 0, m_opacity);
    painter.setBrush(overlayColor);
    QRect r;
    if (m_selection->isVisible()) {
        r = m_selection->geometry().normalized().adjusted(0, 0, -1, -1);
    }
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter.setClipRegion(grey);
    painter.drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter.setClipRect(rect());

    if (m_showInitialMsg) {
        QRect helpRect = QGuiApplication::primaryScreen()->geometry();
        helpRect.moveTo(mapFromGlobal(helpRect.topLeft()));

        QString helpTxt =
          tr("Select an area with the mouse, or press Esc to exit."
             "\nPress Enter to capture the screen."
             "\nPress Right Click to show the color picker."
             "\nUse the Mouse Wheel to change the thickness of your tool."
             "\nPress Space to open the side panel.");

        // We draw the white contrasting background for the text, using the
        // same text and options to get the boundingRect that the text will
        // have.
        QRectF bRect = painter.boundingRect(helpRect, Qt::AlignCenter, helpTxt);

        // These four calls provide padding for the rect
        const int margin = QApplication::fontMetrics().height() / 2;
        bRect.setWidth(bRect.width() + margin);
        bRect.setHeight(bRect.height() + margin);
        bRect.setX(bRect.x() - margin);
        bRect.setY(bRect.y() - margin);

        QColor rectColor(m_uiColor);
        rectColor.setAlpha(180);
        QColor textColor(
          (ColorUtils::colorIsDark(rectColor) ? Qt::white : Qt::black));

        painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
        painter.setPen(QPen(textColor));

        painter.drawRect(bRect);
        painter.drawText(helpRect, Qt::AlignCenter, helpTxt);
    }

    if (m_selection->isVisible()) {
        // paint handlers
        painter.setPen(m_uiColor);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(m_uiColor);
        for (auto r : m_selection->handlerAreas()) {
            painter.drawRoundedRect(r, 100, 100);
        }
    }
}

void CaptureWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton) {
        m_rightClick = true;
        m_colorPicker->move(e->pos().x() - m_colorPicker->width() / 2,
                            e->pos().y() - m_colorPicker->height() / 2);
        m_colorPicker->raise();
        m_colorPicker->show();
    } else if (e->button() == Qt::LeftButton) {
        m_showInitialMsg = false;
        m_mouseIsClicked = true;
        // Click using a tool
        if (m_activeButton) {
            if (m_activeTool) {
                if (m_activeTool->isValid() && m_toolWidget) {
                    pushToolToStack();
                } else {
                    m_activeTool->deleteLater();
                }
                if (m_toolWidget) {
                    m_toolWidget->deleteLater();
                    return;
                }
            }
            m_activeTool = m_activeButton->tool()->copy(this);

            connect(this,
                    &CaptureWidget::colorChanged,
                    m_activeTool,
                    &CaptureTool::colorChanged);
            connect(this,
                    &CaptureWidget::thicknessChanged,
                    m_activeTool,
                    &CaptureTool::thicknessChanged);
            connect(m_activeTool,
                    &CaptureTool::requestAction,
                    this,
                    &CaptureWidget::handleButtonSignal);
            m_activeTool->drawStart(m_context);
            return;
        }

        m_dragStartPoint = e->pos();
        m_selection->saveGeometry();
        // New selection
        if (!m_selection->geometry().contains(e->pos()) &&
            m_mouseOverHandle == SelectionWidget::NO_SIDE) {
            m_selection->setGeometry(QRect(e->pos(), e->pos()));
            m_selection->setVisible(false);
            m_newSelection = true;
            m_buttonHandler->hide();
            update();
        } else {
            m_grabbing = true;
        }
    }
    updateCursor();
}

void CaptureWidget::mouseMoveEvent(QMouseEvent* e)
{
    m_context.mousePos = e->pos();

    if (m_mouseIsClicked && !m_activeButton) {
        if (m_buttonHandler->isVisible()) {
            m_buttonHandler->hide();
        }
        if (m_newSelection) {
            m_selection->setVisible(true);
            m_selection->setGeometry(
              QRect(m_dragStartPoint, m_context.mousePos).normalized());
            update();
        } else if (m_mouseOverHandle == SelectionWidget::NO_SIDE) {
            // Moving the whole selection
            QRect initialRect = m_selection->savedGeometry().normalized();
            QPoint newTopLeft =
              initialRect.topLeft() + (e->pos() - m_dragStartPoint);
            QRect finalRect(newTopLeft, initialRect.size());

            if (finalRect.left() < rect().left()) {
                finalRect.setLeft(rect().left());
            } else if (finalRect.right() > rect().right()) {
                finalRect.setRight(rect().right());
            }
            if (finalRect.top() < rect().top()) {
                finalRect.setTop(rect().top());
            } else if (finalRect.bottom() > rect().bottom()) {
                finalRect.setBottom(rect().bottom());
            }
            m_selection->setGeometry(
              finalRect.normalized().intersected(rect()));
            update();
        } else {
            // Dragging a handle
            QRect r = m_selection->savedGeometry();
            QPoint offset = e->pos() - m_dragStartPoint;
            bool symmetryMod = qApp->keyboardModifiers() & Qt::ShiftModifier;

            using sw = SelectionWidget;
            if (m_mouseOverHandle == sw::TOPLEFT_SIDE ||
                m_mouseOverHandle == sw::TOP_SIDE ||
                m_mouseOverHandle ==
                  sw::TOPRIGHT_SIDE) { // dragging one of the top handles
                r.setTop(r.top() + offset.y());
                if (symmetryMod) {
                    r.setBottom(r.bottom() - offset.y());
                }
            }
            if (m_mouseOverHandle == sw::TOPLEFT_SIDE ||
                m_mouseOverHandle == sw::LEFT_SIDE ||
                m_mouseOverHandle ==
                  sw::BOTTONLEFT_SIDE) { // dragging one of the left handles
                r.setLeft(r.left() + offset.x());
                if (symmetryMod) {
                    r.setRight(r.right() - offset.x());
                }
            }
            if (m_mouseOverHandle == sw::BOTTONLEFT_SIDE ||
                m_mouseOverHandle == sw::BOTTON_SIDE ||
                m_mouseOverHandle ==
                  sw::BOTTONRIGHT_SIDE) { // dragging one of the bottom handles
                r.setBottom(r.bottom() + offset.y());
                if (symmetryMod) {
                    r.setTop(r.top() - offset.y());
                }
            }
            if (m_mouseOverHandle == sw::TOPRIGHT_SIDE ||
                m_mouseOverHandle == sw::RIGHT_SIDE ||
                m_mouseOverHandle ==
                  sw::BOTTONRIGHT_SIDE) { // dragging one of the right handles
                r.setRight(r.right() + offset.x());
                if (symmetryMod) {
                    r.setLeft(r.left() - offset.x());
                }
            }
            m_selection->setGeometry(r.intersected(rect()).normalized());
            update();
        }
    } else if (m_mouseIsClicked && m_activeTool) {
        // drawing with a tool
        if (m_adjustmentButtonPressed) {
            m_activeTool->drawMoveWithAdjustment(e->pos());
        } else {
            m_activeTool->drawMove(e->pos());
        }
        update();
        // Hides the buttons under the mouse. If the mouse leaves, it shows
        // them.
        if (m_buttonHandler->buttonsAreInside()) {
            const bool containsMouse =
              m_buttonHandler->contains(m_context.mousePos);
            if (containsMouse) {
                m_buttonHandler->hide();
            } else {
                m_buttonHandler->show();
            }
        }
    } else if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    } else {
        if (!m_selection->isVisible()) {
            return;
        }
        m_mouseOverHandle = m_selection->getMouseSide(m_context.mousePos);
        updateCursor();
    }
}

void CaptureWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton || m_colorPicker->isVisible()) {
        m_colorPicker->hide();
        m_rightClick = false;
        if (!m_context.color.isValid()) {
            m_panel->show();
        }
        // when we end the drawing we have to register the last  point and
        // add the temp modification to the list of modifications
    } else if (m_mouseIsClicked && m_activeTool) {
        m_activeTool->drawEnd(m_context.mousePos);
        if (m_activeTool->isValid()) {
            pushToolToStack();
        } else if (!m_toolWidget) {
            m_activeTool->deleteLater();
            m_activeTool = nullptr;
        }
    }

    // Show the buttons after the resize of the selection or the creation
    // of a new one.
    if (!m_buttonHandler->isVisible() && m_selection->isVisible()) {
        // Don't go outside
        QRect newGeometry = m_selection->geometry().intersected(rect());
        // normalize
        if (newGeometry.width() <= 0) {
            int left = newGeometry.left();
            newGeometry.setLeft(newGeometry.right());
            newGeometry.setRight(left);
        }
        if (newGeometry.height() <= 0) {
            int top = newGeometry.top();
            newGeometry.setTop(newGeometry.bottom());
            newGeometry.setBottom(top);
        }
        m_selection->setGeometry(newGeometry);
        m_context.selection = extendedRect(&newGeometry);
        updateSizeIndicator();
        m_buttonHandler->updatePosition(newGeometry);
        m_buttonHandler->show();
    }
    m_mouseIsClicked = false;
    m_newSelection = false;
    m_grabbing = false;

    updateCursor();
}

void CaptureWidget::leftMove()
{
    if (m_selection->geometry().left() > rect().left()) {
        m_selection->move(QPoint(m_selection->x() - 1, m_selection->y()));
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    }
}

void CaptureWidget::rightMove()
{
    if (m_selection->geometry().right() < rect().right()) {
        m_selection->move(QPoint(m_selection->x() + 1, m_selection->y()));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    }
}

void CaptureWidget::upMove()
{
    if (m_selection->geometry().top() > rect().top()) {
        m_selection->move(QPoint(m_selection->x(), m_selection->y() - 1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    }
}

void CaptureWidget::downMove()
{
    if (m_selection->geometry().bottom() < rect().bottom()) {
        m_selection->move(QPoint(m_selection->x(), m_selection->y() + 1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        update();
    }
}

void CaptureWidget::keyPressEvent(QKeyEvent* e)
{
    if (!m_selection->isVisible()) {
        return;
    } else if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = true;
    } else if (e->key() == Qt::Key_Enter) {
        // Make no difference for Return and Enter keys
        QKeyEvent* keyReturn =
          new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QCoreApplication::postEvent(this, keyReturn);
    }
}

void CaptureWidget::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = false;
    }
}

void CaptureWidget::wheelEvent(QWheelEvent* e)
{
    m_context.thickness += e->angleDelta().y() / 120;
    m_context.thickness = qBound(0, m_context.thickness, 100);
    QPoint topLeft =
      qApp->desktop()
        ->screenGeometry(qApp->desktop()->screenNumber(QCursor::pos()))
        .topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_context.thickness));
    if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    }
    emit thicknessChanged(m_context.thickness);
}

void CaptureWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    m_context.widgetDimensions = rect();
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    if (!m_context.fullscreen) {
        m_panel->setFixedHeight(height());
        m_buttonHandler->updateScreenRegions(rect());
    }
}

void CaptureWidget::moveEvent(QMoveEvent* e)
{
    QWidget::moveEvent(e);
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
}

void CaptureWidget::initContext(const QString& savePath, bool fullscreen)
{
    m_context.widgetDimensions = rect();
    m_context.color = m_config.drawColorValue();
    m_context.savePath = savePath;
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    m_context.mousePos = mapFromGlobal(QCursor::pos());
    m_context.thickness = m_config.drawThicknessValue();
    m_context.fullscreen = fullscreen;
}

void CaptureWidget::initPanel()
{
    QRect panelRect = rect();
    if (m_context.fullscreen) {
        panelRect = QGuiApplication::primaryScreen()->geometry();
        auto devicePixelRatio =
          QGuiApplication::primaryScreen()->devicePixelRatio();
        panelRect.moveTo(panelRect.x() / devicePixelRatio,
                         panelRect.y() / devicePixelRatio);
    }

    ConfigHandler config;

    if (config.showSidePanelButtonValue()) {
        auto* panelToggleButton =
          new OrientablePushButton(tr("Tool Settings"), this);
        makeChild(panelToggleButton);
        panelToggleButton->setColor(m_uiColor);
        panelToggleButton->setOrientation(
          OrientablePushButton::VerticalBottomToTop);
        panelToggleButton->move(panelRect.x(),
                                panelRect.y() + panelRect.height() / 2 -
                                  panelToggleButton->width() / 2);
        panelToggleButton->setCursor(Qt::ArrowCursor);
        (new DraggableWidgetMaker(this))->makeDraggable(panelToggleButton);
        connect(panelToggleButton,
                &QPushButton::clicked,
                this,
                &CaptureWidget::togglePanel);
    }

    m_panel = new UtilityPanel(this);
    makeChild(m_panel);
    panelRect.moveTo(mapFromGlobal(panelRect.topLeft()));
    panelRect.setWidth(m_colorPicker->width() * 1.5);
    m_panel->setGeometry(panelRect);

    SidePanelWidget* sidePanel = new SidePanelWidget(&m_context.screenshot);
    connect(sidePanel,
            &SidePanelWidget::colorChanged,
            this,
            &CaptureWidget::setDrawColor);
    connect(sidePanel,
            &SidePanelWidget::thicknessChanged,
            this,
            &CaptureWidget::setDrawThickness);
    connect(this,
            &CaptureWidget::colorChanged,
            sidePanel,
            &SidePanelWidget::updateColor);
    connect(this,
            &CaptureWidget::thicknessChanged,
            sidePanel,
            &SidePanelWidget::updateThickness);
    connect(
      sidePanel, &SidePanelWidget::togglePanel, m_panel, &UtilityPanel::toggle);
    sidePanel->colorChanged(m_context.color);
    sidePanel->thicknessChanged(m_context.thickness);
    m_panel->pushWidget(sidePanel);
    m_panel->pushWidget(new QUndoView(&m_undoStack, this));
}

void CaptureWidget::initSelection()
{
    m_selection = new SelectionWidget(m_uiColor, this);
    connect(m_selection, &SelectionWidget::animationEnded, this, [this]() {
        this->m_buttonHandler->updatePosition(this->m_selection->geometry());
    });
    m_selection->setVisible(false);
    m_selection->setGeometry(QRect());
}

void CaptureWidget::setState(CaptureToolButton* b)
{
    if (!b) {
        return;
    }
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
        if (m_activeTool->isValid()) {
            pushToolToStack();
        }
    }
    if (m_activeButton != b) {
        processTool(b->tool());
    }
    // Only close activated from button
    if (b->tool()->closeOnButtonPressed()) {
        close();
    }

    if (b->tool()->isSelectable()) {
        if (m_activeButton != b) {
            QWidget* confW = b->tool()->configurationWidget();
            m_panel->addToolWidget(confW);
            if (m_activeButton) {
                m_activeButton->setColor(m_uiColor);
            }
            m_activeButton = b;
            m_activeButton->setColor(m_contrastUiColor);
        } else if (m_activeButton) {
            m_panel->clearToolWidget();
            m_activeButton->setColor(m_uiColor);
            m_activeButton = nullptr;
        }
        updateCursor();
        update(); // clear mouse preview
    }
}

void CaptureWidget::processTool(CaptureTool* t)
{
    auto backup = m_activeTool;
    // The tool is active during the pressed().
    m_activeTool = t;
    t->pressed(m_context);
    m_activeTool = backup;
}

void CaptureWidget::handleButtonSignal(CaptureTool::Request r)
{
    switch (r) {
        case CaptureTool::REQ_CLEAR_MODIFICATIONS:
            m_undoStack.setIndex(0);
            update();
            break;

        case CaptureTool::REQ_INCREMENT_CIRCLE_COUNT:
            incrementCircleCount();
            break;

        case CaptureTool::REQ_DECREMENT_CIRCLE_COUNT:
            decrementCircleCount();
            break;

        case CaptureTool::REQ_CLOSE_GUI:
            close();
            break;
        case CaptureTool::REQ_HIDE_GUI:
            hide();
            break;
        case CaptureTool::REQ_HIDE_SELECTION:
            m_newSelection = true;
            m_selection->setVisible(false);
            updateCursor();
            break;
        case CaptureTool::REQ_SELECT_ALL:
            m_selection->setGeometryAnimated(rect());
            break;
        case CaptureTool::REQ_UNDO_MODIFICATION:
            m_undoStack.undo();
            break;
        case CaptureTool::REQ_REDO_MODIFICATION:
            if (m_undoStack.redoText() == "Circle Counter") {
                this->incrementCircleCount();
            }
            m_undoStack.redo();
            break;
        case CaptureTool::REQ_REDRAW:
            update();
            break;
        case CaptureTool::REQ_TOGGLE_SIDEBAR:
            m_panel->toggle();
            break;
        case CaptureTool::REQ_SHOW_COLOR_PICKER:
            // TODO
            break;
        case CaptureTool::REQ_MOVE_MODE:
            setState(m_activeButton); // Disable the actual button
            break;
        case CaptureTool::REQ_CAPTURE_DONE_OK:
            m_captureDone = true;
            break;
        case CaptureTool::REQ_ADD_CHILD_WIDGET:
            if (!m_activeTool) {
                break;
            }
            if (m_toolWidget) {
                m_toolWidget->deleteLater();
            }
            m_toolWidget = m_activeTool->widget();
            if (m_toolWidget) {
                makeChild(m_toolWidget);
                m_toolWidget->move(m_context.mousePos);
                m_toolWidget->show();
                m_toolWidget->setFocus();
            }
            break;
        case CaptureTool::REQ_ADD_CHILD_WINDOW:
            if (!m_activeTool) {
                break;
            } else {
                QWidget* w = m_activeTool->widget();
                connect(
                  this, &CaptureWidget::destroyed, w, &QWidget::deleteLater);
                w->show();
            }
            break;
        case CaptureTool::REQ_ADD_EXTERNAL_WIDGETS:
            if (!m_activeTool) {
                break;
            } else {
                QWidget* w = m_activeTool->widget();
                w->setAttribute(Qt::WA_DeleteOnClose);
                w->show();
            }
            break;
        default:
            break;
    }
}

void CaptureWidget::setDrawColor(const QColor& c)
{
    m_context.color = c;
    if (m_context.color.isValid()) {
        ConfigHandler().setDrawColor(m_context.color);
        emit colorChanged(c);
    }
}

void CaptureWidget::incrementCircleCount()
{
    m_context.circleCount++;
}

void CaptureWidget::decrementCircleCount()
{
    m_context.circleCount--;
}

void CaptureWidget::setDrawThickness(const int& t)
{
    m_context.thickness = qBound(0, t, 100);
    ConfigHandler().setdrawThickness(m_context.thickness);
    emit thicknessChanged(m_context.thickness);
}

void CaptureWidget::leftResize()
{
    if (m_selection->isVisible() &&
        m_selection->geometry().right() > m_selection->geometry().left()) {
        m_selection->setGeometry(m_selection->geometry() +
                                 QMargins(0, 0, -1, 0));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::rightResize()
{
    if (m_selection->isVisible() &&
        m_selection->geometry().right() < rect().right()) {
        m_selection->setGeometry(m_selection->geometry() +
                                 QMargins(0, 0, 1, 0));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::upResize()
{
    if (m_selection->isVisible() &&
        m_selection->geometry().bottom() > m_selection->geometry().top()) {
        m_selection->setGeometry(m_selection->geometry() +
                                 QMargins(0, 0, 0, -1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::downResize()
{
    if (m_selection->isVisible() &&
        m_selection->geometry().bottom() < rect().bottom()) {
        m_selection->setGeometry(m_selection->geometry() +
                                 QMargins(0, 0, 0, 1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::initShortcuts()
{
    QString shortcut = ConfigHandler().shortcut(
      QVariant::fromValue(CaptureToolButton::ButtonType::TYPE_EXIT).toString());
    new QShortcut(QKeySequence(shortcut), this, SLOT(close()));

    shortcut = ConfigHandler().shortcut(
      QVariant::fromValue(CaptureToolButton::ButtonType::TYPE_SAVE).toString());
    new QShortcut(QKeySequence(shortcut), this, SLOT(saveScreenshot()));

    shortcut = ConfigHandler().shortcut(
      QVariant::fromValue(CaptureToolButton::ButtonType::TYPE_COPY).toString());
    new QShortcut(QKeySequence(shortcut), this, SLOT(copyScreenshot()));

    shortcut = ConfigHandler().shortcut(
      QVariant::fromValue(CaptureToolButton::ButtonType::TYPE_UNDO).toString());
    new QShortcut(QKeySequence(shortcut), this, SLOT(undo()));

    shortcut = ConfigHandler().shortcut(
      QVariant::fromValue(CaptureToolButton::ButtonType::TYPE_REDO).toString());
    new QShortcut(QKeySequence(shortcut), this, SLOT(redo()));

    shortcut = ConfigHandler().shortcut(
      QVariant::fromValue(CaptureToolButton::ButtonType::TYPE_IMAGEUPLOADER)
        .toString());
    new QShortcut(shortcut, this, SLOT(uploadScreenshot()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_TOGGLE_PANEL")),
                  this,
                  SLOT(togglePanel()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_LEFT")),
                  this,
                  SLOT(leftResize()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_RIGHT")),
                  this,
                  SLOT(rightResize()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_UP")),
                  this,
                  SLOT(upResize()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_DOWN")),
                  this,
                  SLOT(downResize()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_LEFT")),
                  this,
                  SLOT(leftMove()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_RIGHT")),
                  this,
                  SLOT(rightMove()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_UP")),
                  this,
                  SLOT(upMove()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_DOWN")),
                  this,
                  SLOT(downMove()));

    new QShortcut(Qt::Key_Escape, this, SLOT(deleteToolwidgetOrClose()));
}

void CaptureWidget::updateSizeIndicator()
{
    if (m_sizeIndButton) {
        const QRect& selection = extendedSelection();
        m_sizeIndButton->setText(QStringLiteral("%1\n%2")
                                   .arg(selection.width())
                                   .arg(selection.height()));
    }
}

void CaptureWidget::updateCursor()
{
    if (m_rightClick) {
        setCursor(Qt::ArrowCursor);
    } else if (m_grabbing) {
        setCursor(Qt::ClosedHandCursor);
    } else if (!m_activeButton) {
        using sw = SelectionWidget;
        if (m_mouseOverHandle != sw::NO_SIDE) {
            // cursor on the handlers
            switch (m_mouseOverHandle) {
                case sw::TOPLEFT_SIDE:
                case sw::BOTTONRIGHT_SIDE:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case sw::TOPRIGHT_SIDE:
                case sw::BOTTONLEFT_SIDE:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                case sw::LEFT_SIDE:
                case sw::RIGHT_SIDE:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case sw::TOP_SIDE:
                case sw::BOTTON_SIDE:
                    setCursor(Qt::SizeVerCursor);
                    break;
                default:
                    break;
            }
        } else if (m_selection->isVisible() &&
                   m_selection->geometry().contains(m_context.mousePos)) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
    } else {
        setCursor(Qt::CrossCursor);
    }
}

void CaptureWidget::pushToolToStack()
{
    auto mod = new ModificationCommand(&m_context.screenshot, m_activeTool);
    disconnect(this,
               &CaptureWidget::colorChanged,
               m_activeTool,
               &CaptureTool::colorChanged);
    disconnect(this,
               &CaptureWidget::thicknessChanged,
               m_activeTool,
               &CaptureTool::thicknessChanged);
    if (m_panel->toolWidget()) {
        disconnect(m_panel->toolWidget(), nullptr, m_activeTool, nullptr);
    }
    m_undoStack.push(mod);
    m_activeTool = nullptr;
}

void CaptureWidget::makeChild(QWidget* w)
{
    w->setParent(this);
    w->installEventFilter(m_eventFilter);
}

void CaptureWidget::togglePanel()
{
    m_panel->toggle();
}

void CaptureWidget::childEnter()
{
    m_previewEnabled = false;
    update();
}

void CaptureWidget::childLeave()
{
    m_previewEnabled = true;
    update();
}

void CaptureWidget::uploadScreenshot()
{
    StorageManager storageManager;
    m_activeTool =
      storageManager.imgUploaderTool(ConfigHandler().uploadStorage());
    m_activeTool->setCapture(pixmap());
    handleButtonSignal(CaptureTool::REQ_ADD_EXTERNAL_WIDGETS);
    close();
}

void CaptureWidget::copyScreenshot()
{
    m_captureDone = true;
    if (m_activeTool != nullptr) {
        QPainter painter(&m_context.screenshot);
        m_activeTool->process(painter, m_context.screenshot, true);
    }

    ScreenshotSaver().saveToClipboard(pixmap());
    close();
}

void CaptureWidget::saveScreenshot()
{
    m_captureDone = true;
    if (m_activeTool != nullptr) {
        QPainter painter(&m_context.screenshot);
        m_activeTool->process(painter, m_context.screenshot, true);
    }
    hide();
    if (m_context.savePath.isEmpty()) {
        ScreenshotSaver().saveToFilesystemGUI(pixmap());
    } else {
        ScreenshotSaver().saveToFilesystem(pixmap(), m_context.savePath, "");
    }
    close();
}

void CaptureWidget::undo()
{
    m_undoStack.undo();
}

void CaptureWidget::redo()
{
    m_undoStack.redo();
}

QRect CaptureWidget::extendedSelection() const
{
    if (!m_selection->isVisible())
        return QRect();
    QRect r = m_selection->geometry();
    return extendedRect(&r);
}

QRect CaptureWidget::extendedRect(QRect* r) const
{
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();
    return QRect(r->left() * devicePixelRatio,
                 r->top() * devicePixelRatio,
                 r->width() * devicePixelRatio,
                 r->height() * devicePixelRatio);
}