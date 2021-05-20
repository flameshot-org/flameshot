// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#include "capturewidget.h"
#include "src/core/controller.h"
#include "src/core/qguiappcurrentscreen.h"
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
#include "src/widgets/panel/utilitypanel.h"
#include "src/widgets/updatenotificationwidget.h"
#include <QApplication>
#include <QDateTime>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <draggablewidgetmaker.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

#define MOUSE_DISTANCE_TO_START_MOVING 3

// CaptureWidget is the main component used to capture the screen. It contains
// an area of selection with its respective buttons.

// enableSaveWindow
CaptureWidget::CaptureWidget(uint id,
                             const QString& savePath,
                             bool fullScreen,
                             QWidget* parent)
  : QWidget(parent)
  , m_mouseIsClicked(false)
  , m_newSelection(false)
  , m_grabbing(false)
  , m_captureDone(false)
  , m_previewEnabled(true)
  , m_adjustmentButtonPressed(false)
  , m_activeButton(nullptr)
  , m_activeTool(nullptr)
  , m_toolWidget(nullptr)
  , m_colorPicker(nullptr)
  , m_mouseOverHandle(SelectionWidget::NO_SIDE)
  , m_id(id)
  , m_lastMouseWheel(0)
  , m_updateNotificationWidget(nullptr)
  , m_activeToolIsMoved(false)
  , m_panel(nullptr)
  , m_sidePanel(nullptr)
  , m_selection(nullptr)
  , m_existingObjectIsChanged(false)
  , m_startMove(false)
{
    m_undoStack.setUndoLimit(ConfigHandler().undoLimit());

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
#if (defined(Q_OS_WIN) || defined(Q_OS_MACOS))
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

#if defined(Q_OS_WIN)
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                       Qt::Popup);

        for (QScreen* const screen : QGuiApplication::screens()) {
            QPoint topLeftScreen = screen->geometry().topLeft();

            if (topLeftScreen.x() < topLeft.x()) {
                topLeft.setX(topLeftScreen.x());
            }
            if (topLeftScreen.y() < topLeft.y()) {
                topLeft.setY(topLeftScreen.y());
            }
        }
        move(topLeft);
        resize(pixmap().size());
#elif defined(Q_OS_MACOS)
        // Emulate fullscreen mode
        //        setWindowFlags(Qt::WindowStaysOnTopHint |
        //        Qt::BypassWindowManagerHint |
        //                       Qt::FramelessWindowHint |
        //                       Qt::NoDropShadowWindowHint | Qt::ToolTip |
        //                       Qt::Popup
        //                       );
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        move(currentScreen->geometry().x(), currentScreen->geometry().y());
        resize(currentScreen->size());
#else
        // Comment For CaptureWidget Debugging under Linux
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint |
                       Qt::FramelessWindowHint | Qt::Tool);
        resize(pixmap().size());
#endif
    }
    // Create buttons
    m_buttonHandler = new ButtonHandler(this);
    updateButtons();
    QVector<QRect> areas;
    if (m_context.fullscreen) {
        QPoint topLeftOffset = QPoint(0, 0);
#if defined(Q_OS_WIN)
        topLeftOffset = topLeft;
#endif

#if defined(Q_OS_MACOS)
        // MacOS works just with one active display, so we need to append
        // just one current display and keep multiple displays logic for
        // other OS
        QRect r;
        QScreen* screen = QGuiAppCurrentScreen().currentScreen();
        r = screen->geometry();
        // all calculations are processed according to (0, 0) start
        // point so we need to move current object to (0, 0)
        r.moveTo(0, 0);
        areas.append(r);
#else
        for (QScreen* const screen : QGuiApplication::screens()) {
            QRect r = screen->geometry();
            r.moveTo(r.x() / screen->devicePixelRatio(),
                     r.y() / screen->devicePixelRatio());
            r.moveTo(r.topLeft() - topLeftOffset);
            areas.append(r);
        }
#endif
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

    initPanel();
}

CaptureWidget::~CaptureWidget()
{
    if (m_captureDone) {
        emit captureTaken(m_id, this->pixmap(), m_context.selection);
    } else {
        emit captureFailed(m_id);
    }
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
                        captureWidget->setState(b);
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
        painter.setRenderHint(QPainter::Antialiasing);
        m_activeTool->process(painter, p);
    } else {
        p = m_context.selectedScreenshotArea();
    }
    return m_context.selectedScreenshotArea();
}

// Finish whatever the current tool is doing, if there is a current active
// tool.
bool CaptureWidget::commitCurrentTool()
{
    if (m_activeTool) {
        if (m_activeTool->isValid() && !m_activeTool->editMode() &&
            m_toolWidget) {
            pushToolToStack();
        }
        releaseActiveTool();
        return true;
    }
    return false;
}

void CaptureWidget::deleteToolWidgetOrClose()
{
    if (!m_activeButton.isNull()) {
        uncheckActiveTool();
    } else if (m_panel->activeLayerIndex() >= 0) {
        // remove active tool selection
        m_panel->setActiveLayer(-1);
        drawToolsData(false, true);
    } else if (m_panel->isVisible()) {
        // hide panel if visible
        m_panel->hide();
    } else if (m_toolWidget) {
        // delete toolWidget if exists
        m_toolWidget->close();
        delete m_toolWidget;
        m_toolWidget = nullptr;
    } else if (m_colorPicker && m_colorPicker->isVisible()) {
        m_colorPicker->hide();
    } else {
        // close CaptureWidget
        close();
    }
}

void CaptureWidget::releaseActiveTool()
{
    if (m_activeTool) {
        if (m_activeTool->editMode()) {
            // Object shouldn't be deleted here because it is in the undo/redo
            // stack, just set current pointer to null
            m_activeTool->setEditMode(false);
            if (m_activeTool->isChanged()) {
                pushObjectsStateToUndoStack();
            }
        } else {
            delete m_activeTool;
        }
        m_activeTool = nullptr;
    }
    if (m_toolWidget) {
        m_toolWidget->close();
        delete m_toolWidget;
        m_toolWidget = nullptr;
    }
}

void CaptureWidget::uncheckActiveTool()
{
    // uncheck active tool
    m_panel->setToolWidget(nullptr);
    m_activeButton->setColor(m_uiColor);
    m_activeButton = nullptr;
    releaseActiveTool();
    updateCursor();
    update(); // clear mouse preview
}

void CaptureWidget::paintEvent(QPaintEvent* paintEvent)
{
    Q_UNUSED(paintEvent)
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_context.screenshot);

    if (m_activeTool && m_mouseIsClicked) {
        painter.save();
        m_activeTool->process(painter, m_context.screenshot);
        painter.restore();
    } else if (m_previewEnabled && m_activeButton && m_activeButton->tool() &&
               m_activeButton->tool()->showMousePreview()) {
        painter.save();
        m_activeButton->tool()->paintMousePreview(painter, m_context);
        painter.restore();
    }

    // draw inactive region
    drawInactiveRegion(&painter);

    // show initial message on screen capture call if required (before selecting
    // area)
    if (m_showInitialMsg) {
        drawInitialMessage(&painter);
    }
}

void CaptureWidget::showColorPicker(const QPoint& pos)
{
    // Try to select new object if current pos out of active object
    auto toolItem = activeToolObject();
    if (!toolItem || toolItem && !toolItem->selectionRect().contains(pos)) {
        selectToolItemAtPos(pos);
    }

    // save current state for undo/redo stack
    if (m_panel->activeLayerIndex() >= 0) {
        m_captureToolObjectsBackup = m_captureToolObjects;
    }

    // Call color picker
    m_colorPicker->move(pos.x() - m_colorPicker->width() / 2,
                        pos.y() - m_colorPicker->height() / 2);
    m_colorPicker->raise();
    m_colorPicker->show();
}

bool CaptureWidget::startDrawObjectTool(const QPoint& pos)
{
    if (m_activeButton && m_activeButton->tool() &&
        m_activeButton->tool()->nameID() != ToolType::MOVE) {
        if (commitCurrentTool()) {
            return false;
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
        m_context.mousePos = pos;
        m_activeTool->drawStart(m_context);
        if (m_activeTool->nameID() == ToolType::CIRCLECOUNT) {
            // While it is based on AbstractTwoPointTool it has the only one
            // point and shouldn't wait for second point and move event
            m_activeTool->drawEnd(m_context.mousePos);

            m_captureToolObjectsBackup = m_captureToolObjects;
            m_captureToolObjects.append(m_activeTool);
            pushObjectsStateToUndoStack();
            releaseActiveTool();
            drawToolsData();

            m_mouseIsClicked = false;
        }
        return true;
    }
    return false;
}

void CaptureWidget::pushObjectsStateToUndoStack()
{
    m_undoStack.push(new ModificationCommand(
      this, m_captureToolObjects, m_captureToolObjectsBackup));
    m_captureToolObjectsBackup.clear();
}

int CaptureWidget::selectToolItemAtPos(const QPoint& pos)
{
    // Try to select existing tool, "-1" - no active tool
    int activeLayerIndex = -1;
    if (m_activeButton.isNull() &&
        m_captureToolObjects.captureToolObjects().size() > 0 &&
        m_selection->getMouseSide(pos) == SelectionWidget::NO_SIDE) {
        auto toolItem = activeToolObject();
        if (!toolItem ||
            (toolItem && !toolItem->selectionRect().contains(pos))) {
            activeLayerIndex = m_captureToolObjects.find(pos, size());
            int thickness_old = m_context.thickness;
            m_panel->setActiveLayer(activeLayerIndex);
            drawObjectSelection();
            if (thickness_old != m_context.thickness) {
                emit thicknessChanged(m_context.thickness);
            }
        }
    }
    return activeLayerIndex;
}

void CaptureWidget::mousePressEvent(QMouseEvent* e)
{
    m_startMove = false;
    m_startMovePos = QPoint();
    m_dragStartPoint = m_mousePressedPos = e->pos();
    m_activeToolOffsetToMouseOnStart = QPoint();
    if (m_colorPicker->isVisible()) {
        updateCursor();
        return;
    }

    // reset object selection if capture area selection is active
    if (m_selection->getMouseSide(e->pos()) != SelectionWidget::NO_SIDE) {
        m_panel->setActiveLayer(-1);
    }

    if (e->button() == Qt::RightButton) {
        if (m_activeTool && m_activeTool->editMode()) {
            return;
        }
        showColorPicker(m_mousePressedPos);
        return;
    } else if (e->button() == Qt::LeftButton) {
        m_showInitialMsg = false;
        m_mouseIsClicked = true;

        // Click using a tool excluding tool MOVE
        if (startDrawObjectTool(m_mousePressedPos)) {
            // return if success
            return;
        }

        m_selection->saveGeometry();
        // New selection
        if (m_captureToolObjects.captureToolObjects().size() == 0) {
            if (!m_selection->geometry().contains(e->pos()) &&
                m_mouseOverHandle == SelectionWidget::NO_SIDE) {
                m_selection->setGeometry(
                  QRect(m_mousePressedPos, m_mousePressedPos));
                m_selection->setVisible(false);
                m_newSelection = true;
                m_buttonHandler->hide();
                update();
            } else {
                m_grabbing = true;
            }
        }
    }

    // Commit current tool if it has edit widget and mouse click is outside
    // of it
    if (m_toolWidget && !m_toolWidget->geometry().contains(e->pos())) {
        commitCurrentTool();
        m_panel->setToolWidget(nullptr);
        drawToolsData();
        update();
    }

    selectToolItemAtPos(m_mousePressedPos);

    updateCursor();
}

void CaptureWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    int activeLayerIndex = m_panel->activeLayerIndex();
    if (activeLayerIndex != -1) {
        // Start object editing
        auto activeTool = m_captureToolObjects.at(activeLayerIndex);
        if (activeTool && activeTool->nameID() == ToolType::TEXT) {
            m_activeTool = activeTool;
            m_mouseIsClicked = false;
            m_context.mousePos = *m_activeTool->pos();
            m_captureToolObjectsBackup = m_captureToolObjects;
            m_activeTool->setEditMode(true);
            drawToolsData(true, false);
            m_mouseIsClicked = false;
            handleButtonSignal(CaptureTool::REQ_ADD_CHILD_WIDGET);
            m_panel->setToolWidget(m_activeTool->configurationWidget());
        }
    }
}

void CaptureWidget::mouseMoveEvent(QMouseEvent* e)
{
    m_context.mousePos = e->pos();
    bool symmetryMod = qApp->keyboardModifiers() & Qt::ShiftModifier;

    int activeLayerIndex = -1;
    if (m_mouseIsClicked) {
        activeLayerIndex = m_panel->activeLayerIndex();
    }
    if (m_mouseIsClicked && !m_activeButton && activeLayerIndex >= 0) {
        // Move existing object
        if (!m_startMove) {
            // Check for the minimal offset to start moving an object
            if (m_startMovePos.isNull()) {
                m_startMovePos = e->pos();
            }
            if ((e->pos() - m_startMovePos).manhattanLength() >
                MOUSE_DISTANCE_TO_START_MOVING) {
                m_startMove = true;
            }
        }
        if (m_startMove) {
            QPointer<CaptureTool> activeTool =
              m_captureToolObjects.at(activeLayerIndex);
            if (m_activeToolOffsetToMouseOnStart.isNull()) {
                setCursor(Qt::OpenHandCursor);
                m_activeToolOffsetToMouseOnStart =
                  e->pos() - *activeTool->pos();
            }
            if (!m_activeToolIsMoved) {
                // save state before movement for undo stack
                m_captureToolObjectsBackup = m_captureToolObjects;
            }
            m_activeToolIsMoved = true;
            activeTool->move(e->pos() - m_activeToolOffsetToMouseOnStart);
            drawToolsData(false);
        }
    } else if (m_mouseIsClicked &&
               (!m_activeButton ||
                (m_activeButton && m_activeButton->tool() &&
                 m_activeButton->tool()->nameID() == ToolType::MOVE))) {
        // Drawing, moving, or stretching a selection
        m_selection->setVisible(true);
        if (m_buttonHandler->isVisible()) {
            m_buttonHandler->hide();
        }
        QRect inputRect;
        if (m_newSelection) {
            // Drawing a new selection
            inputRect = symmetryMod
                          ? QRect(m_dragStartPoint * 2 - m_context.mousePos,
                                  m_context.mousePos)
                          : QRect(m_dragStartPoint, m_context.mousePos);

        } else if (m_mouseOverHandle == SelectionWidget::NO_SIDE) {
            // Moving the whole selection
            if (m_adjustmentButtonPressed || activeToolObject().isNull()) {
                setCursor(Qt::OpenHandCursor);
                QRect initialRect = m_selection->savedGeometry().normalized();
                QPoint newTopLeft =
                  initialRect.topLeft() + (e->pos() - m_dragStartPoint);
                inputRect = QRect(newTopLeft, initialRect.size());
            } else {
                return;
            }
        } else {
            // Dragging a handle
            inputRect = m_selection->savedGeometry();
            QPoint offset = e->pos() - m_dragStartPoint;

            using sw = SelectionWidget;
            QRect& r = inputRect;
            if (m_mouseOverHandle == sw::TOPLEFT_SIDE ||
                m_mouseOverHandle == sw::TOP_SIDE ||
                m_mouseOverHandle == sw::TOPRIGHT_SIDE) {
                // dragging one of the top handles
                r.setTop(r.top() + offset.y());
                if (symmetryMod) {
                    r.setBottom(r.bottom() - offset.y());
                }
            }
            if (m_mouseOverHandle == sw::TOPLEFT_SIDE ||
                m_mouseOverHandle == sw::LEFT_SIDE ||
                m_mouseOverHandle == sw::BOTTOMLEFT_SIDE) {
                // dragging one of the left handles
                r.setLeft(r.left() + offset.x());
                if (symmetryMod) {
                    r.setRight(r.right() - offset.x());
                }
            }
            if (m_mouseOverHandle == sw::BOTTOMLEFT_SIDE ||
                m_mouseOverHandle == sw::BOTTOM_SIDE ||
                m_mouseOverHandle == sw::BOTTOMRIGHT_SIDE) {
                // dragging one of the bottom handles
                r.setBottom(r.bottom() + offset.y());
                if (symmetryMod) {
                    r.setTop(r.top() - offset.y());
                }
            }
            if (m_mouseOverHandle == sw::TOPRIGHT_SIDE ||
                m_mouseOverHandle == sw::RIGHT_SIDE ||
                m_mouseOverHandle == sw::BOTTOMRIGHT_SIDE) {
                // dragging one of the right handles
                r.setRight(r.right() + offset.x());
                if (symmetryMod) {
                    r.setLeft(r.left() - offset.x());
                }
            }
        }
        m_selection->setGeometry(inputRect.intersected(rect()).normalized());
        update();
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
    } else if (m_activeButton && m_activeButton->tool() &&
               m_activeButton->tool()->showMousePreview()) {
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
    if (e->button() == Qt::LeftButton && m_colorPicker->isVisible()) {
        // Color picker
        if (m_colorPicker->isVisible() && m_panel->activeLayerIndex() >= 0 &&
            m_context.color.isValid()) {
            pushObjectsStateToUndoStack();
        }
        m_colorPicker->hide();
        if (!m_context.color.isValid()) {
            m_context.color = ConfigHandler().drawColorValue();
            m_panel->show();
        }
    } else if (m_mouseIsClicked) {
        if (m_activeTool) {
            // end draw/edit
            m_activeTool->drawEnd(m_context.mousePos);
            if (m_activeTool->isValid()) {
                pushToolToStack();
            } else if (!m_toolWidget) {
                releaseActiveTool();
            }
        } else {
            if (m_activeToolIsMoved) {
                m_activeToolIsMoved = false;
                pushObjectsStateToUndoStack();
            } else if (e->pos() == m_mousePressedPos &&
                       m_activeButton.isNull()) {
                // Try to select existing tool if it was in the selection area
                // but need to select another one
                m_panel->setActiveLayer(
                  m_captureToolObjects.find(e->pos(), size()));
            }
            drawToolsData(true, true);
        }
    }

    if (!m_buttonHandler->isVisible() && m_selection->isVisible()) {
        // Show the buttons after the resize of the selection or the creation
        // of a new one.

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
    m_activeToolIsMoved = false;
    m_newSelection = false;
    m_grabbing = false;

    updateCursor();
}

void CaptureWidget::moveSelection(QPoint p)
{
    adjustSelection(QMargins(-p.x(), -p.y(), p.x(), p.y()));
}

void CaptureWidget::moveLeft()
{
    moveSelection(QPoint(-1, 0));
}

void CaptureWidget::moveRight()
{
    moveSelection(QPoint(1, 0));
}

void CaptureWidget::moveUp()
{
    moveSelection(QPoint(0, -1));
}

void CaptureWidget::moveDown()
{
    moveSelection(QPoint(0, 1));
}

void CaptureWidget::keyPressEvent(QKeyEvent* e)
{
    if (!m_selection->isVisible()) {
        return;
    } else if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = true;
        updateCursor();
    } else if (e->key() == Qt::Key_Enter) {
        // Make no difference for Return and Enter keys
        QCoreApplication::postEvent(
          this,
          new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
    }
}

void CaptureWidget::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = false;
        updateCursor();
    }
}

void CaptureWidget::wheelEvent(QWheelEvent* e)
{
    /* Mouse scroll usually gives value 120, not more or less, just how many
     * times.
     * Touchpad gives the value 2 or more (usually 2-8), it doesn't give
     * too big values like mouse wheel on normal scrolling, so it is almost
     * impossible to scroll. It's easier to calculate number of requests and do
     * not accept events faster that one in 200ms.
     * */
    int thicknessOffset = 0;
    if (e->angleDelta().y() >= 60) {
        // mouse scroll (wheel) increment
        thicknessOffset = 1;
    } else if (e->angleDelta().y() <= -60) {
        // mouse scroll (wheel) decrement
        thicknessOffset = -1;
    } else {
        // touchpad scroll
        qint64 current = QDateTime::currentMSecsSinceEpoch();
        if ((current - m_lastMouseWheel) > 200) {
            if (e->angleDelta().y() > 0) {
                thicknessOffset = 1;
            } else if (e->angleDelta().y() < 0) {
                thicknessOffset = -1;
            }
            m_lastMouseWheel = current;
        } else {
            return;
        }
    }

    m_context.thickness += thicknessOffset;
    m_context.thickness = qBound(1, m_context.thickness, 100);
    QPoint topLeft =
      QGuiAppCurrentScreen().currentScreen()->geometry().topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_context.thickness));
    if (m_activeButton && m_activeButton->tool() &&
        m_activeButton->tool()->showMousePreview()) {
        update();
    }

    // update selected object thickness
    // Reset selection if mouse pos is not in selection area
    auto toolItem = activeToolObject();
    if (toolItem) {
        toolItem->thicknessChanged(m_context.thickness);
        if (!m_existingObjectIsChanged) {
            m_captureToolObjectsBackup = m_captureToolObjects;
            m_existingObjectIsChanged = true;
        }
    }
    emit thicknessChanged(m_context.thickness);
}

void CaptureWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
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
#if (defined(Q_OS_MACOS) || defined(Q_OS_LINUX))
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        panelRect = currentScreen->geometry();
        auto devicePixelRatio = currentScreen->devicePixelRatio();
        panelRect.moveTo(static_cast<int>(panelRect.x() / devicePixelRatio),
                         static_cast<int>(panelRect.y() / devicePixelRatio));
#else
        panelRect = QGuiApplication::primaryScreen()->geometry();
        auto devicePixelRatio =
          QGuiApplication::primaryScreen()->devicePixelRatio();
        panelRect.moveTo(panelRect.x() / devicePixelRatio,
                         panelRect.y() / devicePixelRatio);
#endif
    }

    if (ConfigHandler().showSidePanelButtonValue()) {
        auto* panelToggleButton =
          new OrientablePushButton(tr("Tool Settings"), this);
        makeChild(panelToggleButton);
        panelToggleButton->setColor(m_uiColor);
        panelToggleButton->setOrientation(
          OrientablePushButton::VerticalBottomToTop);
#if defined(Q_OS_MACOS)
        panelToggleButton->move(
          0,
          static_cast<int>(panelRect.height() / 2) -
            static_cast<int>(panelToggleButton->width() / 2));
#else
        panelToggleButton->move(panelRect.x(),
                                panelRect.y() + panelRect.height() / 2 -
                                  panelToggleButton->width() / 2);
#endif
        panelToggleButton->setCursor(Qt::ArrowCursor);
        (new DraggableWidgetMaker(this))->makeDraggable(panelToggleButton);
        connect(panelToggleButton,
                &QPushButton::clicked,
                this,
                &CaptureWidget::togglePanel);
    }

    m_panel = new UtilityPanel(this);
    m_panel->hide();
    makeChild(m_panel);
#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    panelRect.moveTo(mapFromGlobal(panelRect.topLeft()));
    m_panel->setFixedWidth(static_cast<int>(m_colorPicker->width() * 1.5));
    m_panel->setFixedHeight(currentScreen->geometry().height());
#else
    panelRect.moveTo(mapFromGlobal(panelRect.topLeft()));
    panelRect.setWidth(m_colorPicker->width() * 1.5);
    m_panel->setGeometry(panelRect);
#endif
    connect(m_panel,
            &UtilityPanel::layerChanged,
            this,
            &CaptureWidget::updateActiveLayer);

    m_sidePanel = new SidePanelWidget(&m_context.screenshot);
    connect(m_sidePanel,
            &SidePanelWidget::colorChanged,
            this,
            &CaptureWidget::setDrawColor);
    connect(m_sidePanel,
            &SidePanelWidget::thicknessChanged,
            this,
            &CaptureWidget::setDrawThickness);
    connect(this,
            &CaptureWidget::colorChanged,
            m_sidePanel,
            &SidePanelWidget::updateColor);
    connect(this,
            &CaptureWidget::thicknessChanged,
            m_sidePanel,
            &SidePanelWidget::updateThickness);
    connect(m_sidePanel,
            &SidePanelWidget::togglePanel,
            m_panel,
            &UtilityPanel::toggle);
    m_sidePanel->colorChanged(m_context.color);
    m_sidePanel->thicknessChanged(m_context.thickness);
    m_panel->pushWidget(m_sidePanel);

    // Fill undo/redo/history list widget
    m_panel->fillCaptureTools(m_captureToolObjects.captureToolObjects());
}

void CaptureWidget::showAppUpdateNotification(const QString& appLatestVersion,
                                              const QString& appLatestUrl)
{
    if (!ConfigHandler().checkForUpdates()) {
        // option check for updates disabled
        return;
    }
    if (nullptr == m_updateNotificationWidget) {
        m_updateNotificationWidget =
          new UpdateNotificationWidget(this, appLatestVersion, appLatestUrl);
    }
#if defined(Q_OS_MACOS)
    int ax = (width() - m_updateNotificationWidget->width()) / 2;
#elif (defined(Q_OS_LINUX) && QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QRect helpRect = QGuiApplication::primaryScreen()->geometry();
    int ax = helpRect.left() +
             ((helpRect.width() - m_updateNotificationWidget->width()) / 2);
#else
    QRect helpRect;
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        helpRect = currentScreen->geometry();
    } else {
        helpRect = QGuiApplication::primaryScreen()->geometry();
    }
    int ax = helpRect.left() +
             ((helpRect.width() - m_updateNotificationWidget->width()) / 2);
#endif
    m_updateNotificationWidget->move(ax, 0);
    makeChild(m_updateNotificationWidget);
    m_updateNotificationWidget->show();
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

    if (m_toolWidget && m_activeTool) {
        if (m_activeTool->isValid()) {
            pushToolToStack();
        } else {
            releaseActiveTool();
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
            m_panel->setToolWidget(confW);
            if (m_activeButton) {
                m_activeButton->setColor(m_uiColor);
            }
            m_activeButton = b;
            m_activeButton->setColor(m_contrastUiColor);
            m_panel->setActiveLayer(-1);
        } else if (m_activeButton) {
            m_panel->clearToolWidget();
            m_activeButton->setColor(m_uiColor);
            m_activeButton = nullptr;
        }
        loadDrawThickness();
        updateCursor();
        update(); // clear mouse preview
    }
}

void CaptureWidget::loadDrawThickness()
{
    if ((m_activeButton && m_activeButton->tool() &&
         m_activeButton->tool()->nameID() == ToolType::TEXT) ||
        (m_activeTool && m_activeTool->nameID() == ToolType::TEXT)) {
        m_context.thickness = m_config.drawFontSizeValue();
    } else {
        m_context.thickness = m_config.drawThicknessValue();
    }
    m_sidePanel->thicknessChanged(m_context.thickness);
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
            m_captureToolObjects.clear();
            m_undoStack.setIndex(0);
            update();
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
            undo();
            break;
        case CaptureTool::REQ_REDO_MODIFICATION:
            redo();
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
        case CaptureTool::REQ_CLEAR_SELECTION:
            if (m_panel->activeLayerIndex() >= 0) {
                m_panel->setActiveLayer(-1);
                drawToolsData(false, false);
            }
            break;
        case CaptureTool::REQ_CAPTURE_DONE_OK:
            m_captureDone = true;
            break;
        case CaptureTool::REQ_ADD_CHILD_WIDGET:
            if (!m_activeTool) {
                break;
            }
            if (m_toolWidget) {
                m_toolWidget->close();
                delete m_toolWidget;
                m_toolWidget = nullptr;
            }
            m_toolWidget = m_activeTool->widget();
            if (m_toolWidget) {
                makeChild(m_toolWidget);
                m_toolWidget->move(m_context.mousePos);
                m_toolWidget->show();
                m_toolWidget->setFocus();
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
        case CaptureTool::REQ_INCREASE_TOOL_SIZE:
            // increase thickness
            m_context.thickness = qBound(1, m_context.thickness + 1, 100);

            // show notifier circle
            m_notifierBox->showMessage(QString::number(m_context.thickness));

            emit thicknessChanged(m_context.thickness);
            break;
        case CaptureTool::REQ_DECREASE_TOOL_SIZE:
            // decrease thickness
            m_context.thickness = qBound(1, m_context.thickness - 1, 100);

            // show notifier circle
            m_notifierBox->showMessage(QString::number(m_context.thickness));

            emit thicknessChanged(m_context.thickness);
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

        // change color for the active tool
        auto toolItem = activeToolObject();
        if (toolItem) {
            // Change color
            emit toolItem->colorChanged(c);
            drawToolsData(false, true);
        }
    }
}

void CaptureWidget::updateActiveLayer(const int& layer)
{
    // TODO - refactor this part, make all objects to work with
    // m_activeTool->isChanged() and remove m_existingObjectIsChanged
    if (m_activeTool && m_activeTool->nameID() == ToolType::TEXT &&
        m_activeTool->isChanged()) {
        commitCurrentTool();
    }

    if (m_toolWidget) {
        // Release active tool if it is in the editing mode but not changed and
        // has editing widget (ex: text tool)
        releaseActiveTool();
    }

    if (m_existingObjectIsChanged) {
        m_existingObjectIsChanged = false;
        pushObjectsStateToUndoStack();
    }
    drawToolsData(false, true);
}

void CaptureWidget::removeToolObject(int index)
{
    --index;
    if (index >= 0 && index < m_captureToolObjects.size()) {
        const ToolType currentToolType =
          m_captureToolObjects.at(index)->nameID();
        m_captureToolObjectsBackup = m_captureToolObjects;
        m_captureToolObjects.removeAt(index);
        if (currentToolType == ToolType::CIRCLECOUNT) {
            // Do circle count reindex
            int circleCount = 1;
            for (int cnt = 0; cnt < m_captureToolObjects.size(); cnt++) {
                auto toolItem = m_captureToolObjects.at(cnt);
                if (toolItem->nameID() != ToolType::CIRCLECOUNT) {
                    continue;
                }
                if (cnt >= index) {
                    m_captureToolObjects.at(cnt)->setCount(circleCount);
                }
                circleCount++;
            }
        }
        pushObjectsStateToUndoStack();
        drawToolsData();
    }
}

void CaptureWidget::setDrawThickness(const int& t)
{
    m_context.thickness = qBound(1, t, 100);
    // save draw thickness for text and other tool separately
    if (m_activeButton) {
        if (m_activeButton->tool() &&
            m_activeButton->tool()->nameID() == ToolType::TEXT) {
            m_config.setDrawFontSize(m_context.thickness);
        } else {
            m_config.setDrawThickness(m_context.thickness);
        }
    }

    auto toolItem = activeToolObject();
    if (toolItem) {
        // Change thickness
        emit toolItem->thicknessChanged(t);
        drawToolsData(false, true);
    } else {
        emit thicknessChanged(m_context.thickness);
    }
}

void CaptureWidget::repositionSelection(QRect r)
{
    if (m_selection->isVisible()) {
        m_selection->setGeometry(r);
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::adjustSelection(QMargins m)
{
    QRect newGeometry = m_selection->geometry() + m;
    if (rect().contains(newGeometry)) {
        repositionSelection(newGeometry);
    }
}

void CaptureWidget::resizeLeft()
{
    adjustSelection(QMargins(0, 0, -1, 0));
}

void CaptureWidget::resizeRight()
{
    adjustSelection(QMargins(0, 0, 1, 0));
}

void CaptureWidget::resizeUp()
{
    adjustSelection(QMargins(0, 0, 0, -1));
}

void CaptureWidget::resizeDown()
{
    adjustSelection(QMargins(0, 0, 0, 1));
}

void CaptureWidget::selectAll()
{
    QRect newGeometry = rect();
    m_selection->setGeometry(newGeometry);
    m_context.selection = extendedRect(&newGeometry);
    m_selection->setVisible(true);
    m_showInitialMsg = false;
    m_buttonHandler->updatePosition(m_selection->geometry());
    updateSizeIndicator();
    m_buttonHandler->show();
    update();
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

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_TOGGLE_PANEL")),
                  this,
                  SLOT(togglePanel()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_LEFT")),
                  this,
                  SLOT(resizeLeft()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_RIGHT")),
                  this,
                  SLOT(resizeRight()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_UP")),
                  this,
                  SLOT(resizeUp()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_DOWN")),
                  this,
                  SLOT(resizeDown()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_LEFT")),
                  this,
                  SLOT(moveLeft()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_RIGHT")),
                  this,
                  SLOT(moveRight()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_UP")),
                  this,
                  SLOT(moveUp()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_DOWN")),
                  this,
                  SLOT(moveDown()));
    new QShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_DELETE_CURRENT_TOOL")),
      this,
      SLOT(deleteCurrentTool()));

    new QShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_COMMIT_CURRENT_TOOL")),
      this,
      SLOT(commitCurrentTool()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SELECT_ALL")),
                  this,
                  SLOT(selectAll()));

    new QShortcut(Qt::Key_Escape, this, SLOT(deleteToolWidgetOrClose()));
}

void CaptureWidget::deleteCurrentTool()
{
    int thickness_old = m_context.thickness;
    emit m_panel->slotButtonDelete(true);
    drawObjectSelection();
    if (thickness_old != m_context.thickness) {
        emit thicknessChanged(m_context.thickness);
    }
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
    if (m_colorPicker && m_colorPicker->isVisible()) {
        setCursor(Qt::ArrowCursor);
    } else if (m_grabbing) {
        if (m_adjustmentButtonPressed) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else if (m_activeButton && m_activeButton->tool() &&
               m_activeButton->tool()->nameID() == ToolType::MOVE) {
        setCursor(Qt::OpenHandCursor);
    } else if (!m_activeButton) {
        using sw = SelectionWidget;
        if (m_mouseOverHandle != sw::NO_SIDE) {
            // cursor on the handlers
            switch (m_mouseOverHandle) {
                case sw::TOPLEFT_SIDE:
                case sw::BOTTOMRIGHT_SIDE:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case sw::TOPRIGHT_SIDE:
                case sw::BOTTOMLEFT_SIDE:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                case sw::LEFT_SIDE:
                case sw::RIGHT_SIDE:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case sw::TOP_SIDE:
                case sw::BOTTOM_SIDE:
                    setCursor(Qt::SizeVerCursor);
                    break;
                default:
                    break;
            }
        } else if (m_selection->isVisible() &&
                   m_selection->geometry().contains(m_context.mousePos)) {
            if (m_adjustmentButtonPressed) {
                setCursor(Qt::OpenHandCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        } else if (m_selection->isVisible() &&
                   m_captureToolObjects.captureToolObjects().size() > 0 &&
                   m_activeTool.isNull()) {
            setCursor(Qt::ArrowCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
    } else {
        setCursor(Qt::CrossCursor);
    }
}

void CaptureWidget::pushToolToStack()
{
    // append current tool to the new state
    if (m_activeTool && m_activeButton) {
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

        // disable signal connect for updating layer because it may call this
        // function again on text objects
        disconnect(m_panel,
                   &UtilityPanel::layerChanged,
                   this,
                   &CaptureWidget::updateActiveLayer);

        m_captureToolObjectsBackup = m_captureToolObjects;
        m_captureToolObjects.append(m_activeTool);
        pushObjectsStateToUndoStack();
        releaseActiveTool();
        drawToolsData();

        // restore signal connection for updating layer
        connect(m_panel,
                &UtilityPanel::layerChanged,
                this,
                &CaptureWidget::updateActiveLayer);
    }
}

void CaptureWidget::drawToolsData(bool updateLayersPanel, bool drawSelection)
{
    QPixmap pixmapItem = m_context.origScreenshot.copy();
    QPainter painter(&pixmapItem);
    painter.setRenderHint(QPainter::Antialiasing);
    int circleCount = 1;
    for (auto toolItem : m_captureToolObjects.captureToolObjects()) {
        if (toolItem->nameID() == ToolType::CIRCLECOUNT) {
            toolItem->setCount(circleCount++);
        }
        toolItem->process(painter, pixmapItem);
    }

    m_context.screenshot = pixmapItem.copy();
    update();
    if (updateLayersPanel) {
        m_panel->fillCaptureTools(m_captureToolObjects.captureToolObjects());
    }

    if (drawSelection) {
        int thickness_old = m_context.thickness;
        drawObjectSelection();
        if (thickness_old != m_context.thickness) {
            emit thicknessChanged(m_context.thickness);
        }
    }
}

void CaptureWidget::drawObjectSelection()
{
    auto toolItem = activeToolObject();
    if (toolItem && !toolItem->editMode()) {
        QPainter painter(&m_context.screenshot);
        toolItem->drawObjectSelection(painter);
        if (m_context.thickness != toolItem->thickness()) {
            m_context.thickness =
              toolItem->thickness() <= 0 ? 0 : toolItem->thickness();
        }
        if (activeToolObject() && m_activeButton) {
            uncheckActiveTool();
        }
    }
}

QPointer<CaptureTool> CaptureWidget::activeToolObject()
{
    return m_captureToolObjects.at(m_panel->activeLayerIndex());
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

void CaptureWidget::copyScreenshot()
{
    m_captureDone = true;
    if (m_activeTool != nullptr) {
        QPainter painter(&m_context.screenshot);
        painter.setRenderHint(QPainter::Antialiasing);
        m_activeTool->process(painter, m_context.screenshot);
    }

    ScreenshotSaver().saveToClipboard(pixmap());
    close();
}

void CaptureWidget::saveScreenshot()
{
#if defined(Q_OS_MACOS)
    showNormal();
#endif
    m_captureDone = true;
    if (m_activeTool != nullptr) {
        QPainter painter(&m_context.screenshot);
        painter.setRenderHint(QPainter::Antialiasing);
        m_activeTool->process(painter, m_context.screenshot);
    }
    hide();
    if (m_context.savePath.isEmpty()) {
        ScreenshotSaver(m_id).saveToFilesystemGUI(pixmap());
    } else {
        ScreenshotSaver(m_id).saveToFilesystem(
          pixmap(), m_context.savePath, "");
    }
    close();
}

void CaptureWidget::setCaptureToolObjects(
  const CaptureToolObjects& captureToolObjects)
{
    // Used for undo/redo
    m_captureToolObjects = captureToolObjects;
    drawToolsData(true, true);
}

void CaptureWidget::undo()
{
    if (m_activeTool &&
        (m_activeTool->isChanged() || m_activeTool->editMode())) {
        // Remove selection on undo, at the same time commit current tool will
        // be called
        m_panel->setActiveLayer(-1);
    }

    m_undoStack.undo();
    drawToolsData();
}

void CaptureWidget::redo()
{
    m_undoStack.redo();
    drawToolsData();
}

QRect CaptureWidget::extendedSelection() const
{
    if (!m_selection->isVisible()) {
        return QRect();
    }
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

void CaptureWidget::drawInitialMessage(QPainter* painter)
{
    if (nullptr == painter) {
        return;
    }
#if (defined(Q_OS_MACOS) || defined(Q_OS_LINUX))
    QRect helpRect;
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        helpRect = currentScreen->geometry();
    } else {
        helpRect = QGuiApplication::primaryScreen()->geometry();
    }
#else
    QRect helpRect = QGuiApplication::primaryScreen()->geometry();
#endif

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
    QRectF bRect = painter->boundingRect(helpRect, Qt::AlignCenter, helpTxt);

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

    painter->setBrush(QBrush(rectColor, Qt::SolidPattern));
    painter->setPen(QPen(textColor));

    painter->drawRect(bRect);
    painter->drawText(helpRect, Qt::AlignCenter, helpTxt);
}

void CaptureWidget::drawInactiveRegion(QPainter* painter)
{
    QColor overlayColor(0, 0, 0, m_opacity);
    painter->setBrush(overlayColor);
    QRect r;
    if (m_selection->isVisible()) {
        r = m_selection->geometry().normalized().adjusted(0, 0, -1, -1);
    }
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter->setClipRegion(grey);
    painter->drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter->setClipRect(rect());

    if (m_selection->isVisible()) {
        // paint handlers
        painter->setPen(m_uiColor);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(m_uiColor);
        for (auto rect : m_selection->handlerAreas()) {
            painter->drawRoundedRect(rect, 100, 100);
        }
    }
}
