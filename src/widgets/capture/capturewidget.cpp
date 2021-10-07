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
#include "src/widgets/capture/overlaymessage.h"
#include "src/widgets/orientablepushbutton.h"
#include "src/widgets/panel/sidepanelwidget.h"
#include "src/widgets/panel/utilitypanel.h"
#include "src/widgets/updatenotificationwidget.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QFontMetrics>
#include <QLabel>
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
  , m_captureDone(false)
  , m_previewEnabled(true)
  , m_adjustmentButtonPressed(false)
  , m_configError(false)
  , m_configErrorResolved(false)
  , m_activeButton(nullptr)
  , m_activeTool(nullptr)
  , m_toolWidget(nullptr)
  , m_colorPicker(nullptr)
  , m_id(id)
  , m_lastMouseWheel(0)
  , m_updateNotificationWidget(nullptr)
  , m_activeToolIsMoved(false)
  , m_panel(nullptr)
  , m_sidePanel(nullptr)
  , m_selection(nullptr)
  , m_existingObjectIsChanged(false)
  , m_startMove(false)
  , m_thicknessByKeyboard(0)
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
    m_opacity = m_config.contrastOpacity();
    m_uiColor = m_config.uiColor();
    m_contrastUiColor = m_config.contrastUiColor();
    setMouseTracking(true);
    initContext(savePath, fullScreen);
    initSelection();
    initShortcuts(); // must be called after initSelection
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
// Call cmake with -DFLAMESHOT_DEBUG_CAPTURE=true to enable easier debugging
#if !defined(FLAMESHOT_DEBUG_CAPTURE)
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint |
                       Qt::FramelessWindowHint | Qt::Tool);
        resize(pixmap().size());
#endif
#endif
    }
    // Create buttons
    m_buttonHandler = new ButtonHandler(this);
    initButtons();
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
    connect(m_notifierBox, &NotifierBox::hidden, this, [this]() {
        // Show cursor if it was hidden while adjusting tool thickness
        updateCursor();
        m_thicknessByKeyboard = 0;
        setDrawThickness(m_context.thickness);
    });

    initPanel();

    m_config.checkAndHandleError();
    if (m_config.hasError()) {
        m_configError = true;
    }
    connect(ConfigHandler::getInstance(), &ConfigHandler::error, this, [=]() {
        m_configError = true;
        m_configErrorResolved = false;
        OverlayMessage::instance()->update();
    });
    connect(
      ConfigHandler::getInstance(), &ConfigHandler::errorResolved, this, [=]() {
          m_configError = false;
          m_configErrorResolved = true;
          OverlayMessage::instance()->update();
      });

    OverlayMessage::init(this,
                         QGuiAppCurrentScreen().currentScreen()->geometry());

    if (m_config.showHelp()) {
        OverlayMessage::push(
          tr("Select an area with the mouse, or press Esc to exit."
             "\nPress Enter to capture the screen."
             "\nPress Right Click to show the color picker."
             "\nUse the Mouse Wheel to change the thickness of your tool."
             "\nPress Space to open the side panel."));
    }

    updateCursor();
}

CaptureWidget::~CaptureWidget()
{
    if (m_captureDone) {
        emit captureTaken(m_id, this->pixmap(), m_context.selection);
    } else {
        emit captureFailed(m_id);
    }
}

void CaptureWidget::initButtons()
{
    auto allButtonTypes = CaptureToolButton::getIterableButtonTypes();
    auto visibleButtonTypes = m_config.buttons();
    QVector<CaptureToolButton*> vectorButtons;

    // Add all buttons but hide those that were disabled in the Interface config
    // This will allow keyboard shortcuts for those buttons to work
    for (const CaptureTool::Type& t : allButtonTypes) {
        CaptureToolButton* b = new CaptureToolButton(t, this);
        if (t == CaptureTool::TYPE_SELECTIONINDICATOR) {
            m_sizeIndButton = b;
        }
        b->setColor(m_uiColor);
        b->hide();
        // must be enabled for SelectionWidget's eventFilter to work correctly
        b->setAttribute(Qt::WA_NoMousePropagation);
        makeChild(b);

        switch (t) {
            case CaptureTool::TYPE_EXIT:
            case CaptureTool::TYPE_SAVE:
            case CaptureTool::TYPE_COPY:
            case CaptureTool::TYPE_UNDO:
            case CaptureTool::TYPE_IMAGEUPLOADER:
            case CaptureTool::TYPE_REDO:
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

        connect(b->tool(),
                &CaptureTool::requestAction,
                this,
                &CaptureWidget::handleToolSignal);

        if (visibleButtonTypes.contains(t)) {
            connect(b,
                    &CaptureToolButton::pressedButton,
                    this,
                    &CaptureWidget::setState);
            vectorButtons << b;
        }
    }
    m_buttonHandler->setButtons(vectorButtons);
}

QPixmap CaptureWidget::pixmap()
{
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
        if (m_toolWidget) {
            m_toolWidget->update();
        }
        releaseActiveTool();
        return true;
    }
    return false;
}

void CaptureWidget::deleteToolWidgetOrClose()
{
    if (m_activeButton != nullptr) {
        uncheckActiveTool();
    } else if (m_panel->activeLayerIndex() >= 0) {
        // remove active tool selection
        m_panel->setActiveLayer(-1);
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
    updateToolMousePreview(activeButtonTool());
    m_activeButton = nullptr;
    releaseActiveTool();
    updateSelectionState();
    updateCursor();
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
    } else if (m_previewEnabled && activeButtonTool() &&
               m_activeButton->tool()->showMousePreview()) {
        painter.save();
        m_activeButton->tool()->paintMousePreview(painter, m_context);
        painter.restore();
    }

    // draw inactive region
    drawInactiveRegion(&painter);

    if (m_configError || m_configErrorResolved) {
        drawConfigErrorMessage(&painter);
    }
}

void CaptureWidget::showColorPicker(const QPoint& pos)
{
    // Try to select new object if current pos out of active object
    auto toolItem = activeToolObject();
    if (!toolItem || toolItem && !toolItem->boundingRect().contains(pos)) {
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
    if (activeButtonToolType() != CaptureTool::NONE &&
        activeButtonToolType() != CaptureTool::TYPE_MOVESELECTION) {
        if (commitCurrentTool()) {
            return false;
        }
        m_activeTool = m_activeButton->tool()->copy(this);

        connect(this,
                &CaptureWidget::colorChanged,
                m_activeTool,
                &CaptureTool::onColorChanged);
        connect(this,
                &CaptureWidget::thicknessChanged,
                m_activeTool,
                &CaptureTool::onThicknessChanged);
        connect(m_activeTool,
                &CaptureTool::requestAction,
                this,
                &CaptureWidget::handleToolSignal);
        m_context.mousePos = pos;
        m_activeTool->drawStart(m_context);
        // TODO this is the wrong place to do this
        if (m_activeTool->type() == CaptureTool::TYPE_CIRCLECOUNT) {
            // While it is based on AbstractTwoPointTool it has the only one
            // point and shouldn't wait for second point and move event
            m_activeTool->drawEnd(m_context.mousePos);

            m_captureToolObjectsBackup = m_captureToolObjects;
            m_captureToolObjects.append(m_activeTool);
            pushObjectsStateToUndoStack();
            releaseActiveTool();
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
    auto selectionMouseSide = m_selection->getMouseSide(pos);
    if (m_activeButton.isNull() &&
        m_captureToolObjects.captureToolObjects().size() > 0 &&
        (selectionMouseSide == SelectionWidget::NO_SIDE ||
         selectionMouseSide == SelectionWidget::CENTER)) {
        auto toolItem = activeToolObject();
        if (!toolItem ||
            (toolItem && !toolItem->boundingRect().contains(pos))) {
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
    m_mousePressedPos = e->pos();
    m_activeToolOffsetToMouseOnStart = QPoint();
    if (m_colorPicker->isVisible()) {
        updateCursor();
        return;
    }

    // reset object selection if capture area selection is active
    if (m_selection->getMouseSide(e->pos()) != SelectionWidget::CENTER) {
        m_panel->setActiveLayer(-1);
    }

    if (e->button() == Qt::RightButton) {
        if (m_activeTool && m_activeTool->editMode()) {
            return;
        }
        showColorPicker(m_mousePressedPos);
        return;
    } else if (e->button() == Qt::LeftButton) {
        OverlayMessage::pop();
        m_mouseIsClicked = true;

        // Click using a tool excluding tool MOVE
        if (startDrawObjectTool(m_mousePressedPos)) {
            // return if success
            return;
        }
    }

    // Commit current tool if it has edit widget and mouse click is outside
    // of it
    if (m_toolWidget && !m_toolWidget->geometry().contains(e->pos())) {
        commitCurrentTool();
        m_panel->setToolWidget(nullptr);
        drawToolsData();
        updateLayersPanel();
    }

    selectToolItemAtPos(m_mousePressedPos);

    updateSelectionState();
    updateCursor();
}

void CaptureWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    int activeLayerIndex = m_panel->activeLayerIndex();
    if (activeLayerIndex != -1) {
        // Start object editing
        auto activeTool = m_captureToolObjects.at(activeLayerIndex);
        if (activeTool && activeTool->type() == CaptureTool::TYPE_TEXT) {
            m_activeTool = activeTool;
            m_mouseIsClicked = false;
            m_context.mousePos = *m_activeTool->pos();
            m_captureToolObjectsBackup = m_captureToolObjects;
            m_activeTool->setEditMode(true);
            drawToolsData();
            updateLayersPanel();
            handleToolSignal(CaptureTool::REQ_ADD_CHILD_WIDGET);
            m_panel->setToolWidget(m_activeTool->configurationWidget());
        }
    } else if (m_selection->geometry().contains(event->pos())) {
        copyScreenshot();
    }
}

void CaptureWidget::mouseMoveEvent(QMouseEvent* e)
{
    m_context.mousePos = e->pos();
    if (e->buttons() != Qt::LeftButton) {
        updateToolMousePreview(activeButtonTool());
        updateCursor();
        return;
    }

    // The rest assumes that left mouse button is clicked
    if (!m_activeButton && m_panel->activeLayerIndex() >= 0) {
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
              m_captureToolObjects.at(m_panel->activeLayerIndex());
            if (m_activeToolOffsetToMouseOnStart.isNull()) {
                setCursor(Qt::ClosedHandCursor);
                m_activeToolOffsetToMouseOnStart =
                  e->pos() - *activeTool->pos();
            }
            if (!m_activeToolIsMoved) {
                // save state before movement for undo stack
                m_captureToolObjectsBackup = m_captureToolObjects;
            }
            m_activeToolIsMoved = true;
            // update the old region of the selection, margins are added to
            // ensure selection outline is updated too
            update(paddedUpdateRect(activeTool->boundingRect()));
            activeTool->move(e->pos() - m_activeToolOffsetToMouseOnStart);
            drawToolsData();
        }
    } else if (m_activeTool) {
        // drawing with a tool
        if (m_adjustmentButtonPressed) {
            m_activeTool->drawMoveWithAdjustment(e->pos());
        } else {
            m_activeTool->drawMove(e->pos());
        }
        // update drawing object
        updateToolMousePreview(m_activeTool);
        // Hides the buttons under the mouse. If the mouse leaves, it shows
        // them.
        if (m_buttonHandler->buttonsAreInside()) {
            const bool containsMouse =
              m_buttonHandler->contains(m_context.mousePos);
            if (containsMouse) {
                m_buttonHandler->hide();
            } else if (m_selection->isVisible()) {
                m_buttonHandler->show();
            }
        }
    }
    updateCursor();
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
            m_context.color = ConfigHandler().drawColor();
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
            }
        }
    }
    m_mouseIsClicked = false;
    m_activeToolIsMoved = false;

    updateSelectionState();
    updateCursor();
}

void CaptureWidget::updateThickness(int thickness)
{
    auto tool = activeButtonTool();
    updateToolMousePreview(tool);
    m_context.thickness = qBound(1, thickness, maxDrawThickness);

    QPoint topLeft =
      QGuiAppCurrentScreen().currentScreen()->geometry().topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_context.thickness));

    if (tool && tool->showMousePreview()) {
        setCursor(Qt::BlankCursor);
        updateToolMousePreview(tool);
    }

    // update selected object thickness
    auto toolItem = activeToolObject();
    if (toolItem) {
        toolItem->onThicknessChanged(m_context.thickness);
        if (!m_existingObjectIsChanged) {
            m_captureToolObjectsBackup = m_captureToolObjects;
            m_existingObjectIsChanged = true;
        }
    }
    emit thicknessChanged(m_context.thickness);
}

void CaptureWidget::keyPressEvent(QKeyEvent* e)
{
    // If the key is a digit, change the thickness
    bool ok;
    int digit = e->text().toInt(&ok);
    if (ok && e->modifiers() == Qt::NoModifier) { // digit received
        m_thicknessByKeyboard = 10 * m_thicknessByKeyboard + digit;
        updateThickness(m_thicknessByKeyboard);
        if (m_context.thickness != m_thicknessByKeyboard) {
            // The thickness was out of range and was clipped by updateThickness
            m_thicknessByKeyboard = 0;
        }
    } else {
        m_thicknessByKeyboard = 0;
    }

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

    updateThickness(m_context.thickness + thicknessOffset);
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
    m_context.color = m_config.drawColor();
    m_context.savePath = savePath;
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    m_context.mousePos = mapFromGlobal(QCursor::pos());
    m_context.thickness = m_config.drawThickness();
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

    if (ConfigHandler().showSidePanelButton()) {
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

    m_sidePanel = new SidePanelWidget(&m_context.screenshot, this);
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
    emit m_sidePanel->colorChanged(m_context.color);
    emit m_sidePanel->thicknessChanged(m_context.thickness);
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
    m_selection->setVisible(false);
    m_selection->setGeometry(QRect());
    connect(m_selection, &SelectionWidget::geometryChanged, this, [this]() {
        m_buttonHandler->updatePosition(m_selection->geometry());
        QRect constrainedToCaptureArea =
          m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(constrainedToCaptureArea);
        updateSizeIndicator();
        m_buttonHandler->hide();
        updateCursor();
    });
    connect(m_selection, &SelectionWidget::geometrySettled, this, [this]() {
        if (m_selection->isVisible()) {
            m_buttonHandler->show();
        } else {
            m_buttonHandler->hide();
        }
    });
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
        auto backup = m_activeTool;
        // The tool is active during the pressed().
        // This must be done in order to handle tool requests correctly.
        m_activeTool = b->tool();
        m_activeTool->pressed(m_context);
        m_activeTool = backup;
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
        updateSelectionState();
        updateToolMousePreview(b->tool());
    }
}

void CaptureWidget::loadDrawThickness()
{
    if ((activeButtonToolType() == CaptureTool::TYPE_TEXT) ||
        (m_activeTool && m_activeTool->type() == CaptureTool::TYPE_TEXT)) {
        m_context.thickness = m_config.drawFontSize();
    } else {
        m_context.thickness = m_config.drawThickness();
    }
    emit m_sidePanel->thicknessChanged(m_context.thickness);
}

void CaptureWidget::handleToolSignal(CaptureTool::Request r)
{
    switch (r) {
        case CaptureTool::REQ_CLOSE_GUI:
            close();
            break;
        case CaptureTool::REQ_HIDE_GUI:
            hide();
            break;
        case CaptureTool::REQ_UNDO_MODIFICATION:
            undo();
            break;
        case CaptureTool::REQ_REDO_MODIFICATION:
            redo();
            break;
        case CaptureTool::REQ_SHOW_COLOR_PICKER:
            // TODO
            break;
        case CaptureTool::REQ_CLEAR_SELECTION:
            if (m_panel->activeLayerIndex() >= 0) {
                m_panel->setActiveLayer(-1);
                drawToolsData();
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
                w->activateWindow();
                w->show();
            }
            break;
        case CaptureTool::REQ_INCREASE_TOOL_SIZE:
            updateThickness(m_context.thickness + 1);
            break;
        case CaptureTool::REQ_DECREASE_TOOL_SIZE:
            updateThickness(m_context.thickness - 1);
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
            toolItem->onColorChanged(c);
            drawToolsData();
        }
    }
}

void CaptureWidget::updateActiveLayer(int layer)
{
    // TODO - refactor this part, make all objects to work with
    // m_activeTool->isChanged() and remove m_existingObjectIsChanged
    if (m_activeTool && m_activeTool->type() == CaptureTool::TYPE_TEXT &&
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
    drawToolsData();
    drawObjectSelection();
    updateSelectionState();
}

void CaptureWidget::selectAll()
{
    m_selection->show();
    m_selection->setGeometry(rect());
    m_buttonHandler->show();
    updateSelectionState();
}

void CaptureWidget::removeToolObject(int index)
{
    --index;
    if (index >= 0 && index < m_captureToolObjects.size()) {
        const CaptureTool::Type currentToolType =
          m_captureToolObjects.at(index)->type();
        m_captureToolObjectsBackup = m_captureToolObjects;
        update(
          paddedUpdateRect(m_captureToolObjects.at(index)->boundingRect()));
        m_captureToolObjects.removeAt(index);
        if (currentToolType == CaptureTool::TYPE_CIRCLECOUNT) {
            // Do circle count reindex
            int circleCount = 1;
            for (int cnt = 0; cnt < m_captureToolObjects.size(); cnt++) {
                auto toolItem = m_captureToolObjects.at(cnt);
                if (toolItem->type() != CaptureTool::TYPE_CIRCLECOUNT) {
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
        updateLayersPanel();
    }
}

void CaptureWidget::setDrawThickness(int t)
{
    m_context.thickness = qBound(1, t, maxDrawThickness);
    // save draw thickness for text and other tool separately
    if (m_activeButton) {
        if (activeButtonToolType() == CaptureTool::TYPE_TEXT) {
            m_config.setDrawFontSize(m_context.thickness);
        } else {
            m_config.setDrawThickness(m_context.thickness);
        }
    }

    auto toolItem = activeToolObject();
    if (toolItem) {
        // Change thickness
        toolItem->onThicknessChanged(t);
        drawToolsData();
        drawObjectSelection();
    } else {
        emit thicknessChanged(m_context.thickness);
    }
}

void CaptureWidget::initShortcuts()
{
    new QShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_EXIT")), this, SLOT(close()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SAVE")),
                  this,
                  SLOT(saveScreenshot()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_COPY")),
                  this,
                  SLOT(copyScreenshot()));

    new QShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_UNDO")), this, SLOT(undo()));

    new QShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_REDO")), this, SLOT(redo()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_TOGGLE_PANEL")),
                  this,
                  SLOT(togglePanel()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_LEFT")),
                  m_selection,
                  SLOT(resizeLeft()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_RIGHT")),
                  m_selection,
                  SLOT(resizeRight()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_UP")),
                  m_selection,
                  SLOT(resizeUp()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_DOWN")),
                  m_selection,
                  SLOT(resizeDown()));

    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_LEFT")),
                  m_selection,
                  SLOT(moveLeft()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_RIGHT")),
                  m_selection,
                  SLOT(moveRight()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_UP")),
                  m_selection,
                  SLOT(moveUp()));
    new QShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_DOWN")),
                  m_selection,
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
    m_panel->slotButtonDelete(true);
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
    } else if (m_activeButton != nullptr &&
               activeButtonToolType() != CaptureTool::TYPE_MOVESELECTION) {
        setCursor(Qt::CrossCursor);
    } else if (m_selection->getMouseSide(mapFromGlobal(QCursor::pos())) !=
               SelectionWidget::NO_SIDE) {
        setCursor(m_selection->cursor());
    } else if (activeButtonToolType() == CaptureTool::TYPE_MOVESELECTION) {
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::CrossCursor);
    }
}

void CaptureWidget::updateSelectionState()
{
    auto toolType = activeButtonToolType();
    if (toolType == CaptureTool::TYPE_MOVESELECTION) {
        m_selection->setIdleCentralCursor(Qt::OpenHandCursor);
        m_selection->setIgnoreMouse(false);
    } else {
        m_selection->setIdleCentralCursor(Qt::ArrowCursor);
        if (toolType == CaptureTool::NONE) {
            m_selection->setIgnoreMouse(m_panel->activeLayerIndex() != -1);
        } else {
            m_selection->setIgnoreMouse(true);
        }
    }
}

void CaptureWidget::updateToolMousePreview(CaptureTool* tool)
{
    if (!tool || !tool->showMousePreview()) {
        return;
    }

    static QRect oldRect;

    QRect r(tool->mousePreviewRect(m_context));
    r += QMargins(r.width(), r.height(), r.width(), r.height());

    QRect toolObjectRect = paddedUpdateRect(tool->boundingRect());

    // oldRect is united with the current rect to handle sudden mouse movement
    update(r.united(oldRect).united(toolObjectRect));
    oldRect = r;
}

void CaptureWidget::updateLayersPanel()
{
    m_panel->fillCaptureTools(m_captureToolObjects.captureToolObjects());
}

void CaptureWidget::pushToolToStack()
{
    // append current tool to the new state
    if (m_activeTool && m_activeButton) {
        disconnect(this,
                   &CaptureWidget::colorChanged,
                   m_activeTool,
                   &CaptureTool::onColorChanged);
        disconnect(this,
                   &CaptureWidget::thicknessChanged,
                   m_activeTool,
                   &CaptureTool::onThicknessChanged);
        if (m_panel->toolWidget()) {
            disconnect(m_panel->toolWidget(), nullptr, m_activeTool, nullptr);
        }

        // disable signal connect for updating layer because it may call this
        // function again on text objects
        m_panel->blockSignals(true);

        m_captureToolObjectsBackup = m_captureToolObjects;
        m_captureToolObjects.append(m_activeTool);
        pushObjectsStateToUndoStack();
        releaseActiveTool();
        drawToolsData();
        updateLayersPanel();

        // restore signal connection for updating layer
        m_panel->blockSignals(false);
    }
}

void CaptureWidget::drawToolsData()
{
    // TODO refactor this for performance. The objects should not all be updated
    // at once every time
    QPixmap pixmapItem = m_context.origScreenshot;
    int circleCount = 1;
    for (auto toolItem : m_captureToolObjects.captureToolObjects()) {
        if (toolItem->type() == CaptureTool::TYPE_CIRCLECOUNT) {
            toolItem->setCount(circleCount++);
        }
        processPixmapWithTool(&pixmapItem, toolItem);
        update(paddedUpdateRect(toolItem->boundingRect()));
    }

    m_context.screenshot = pixmapItem;
    drawObjectSelection();
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

void CaptureWidget::processPixmapWithTool(QPixmap* pixmap, CaptureTool* tool)
{
    QPainter painter(pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    tool->process(painter, *pixmap);
}

CaptureTool* CaptureWidget::activeButtonTool() const
{
    if (m_activeButton == nullptr) {
        return nullptr;
    }
    return m_activeButton->tool();
}

CaptureTool::Type CaptureWidget::activeButtonToolType() const
{
    auto* activeTool = activeButtonTool();
    if (activeTool == nullptr) {
        return CaptureTool::NONE;
    }
    return activeTool->type();
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
    updateToolMousePreview(activeButtonTool());
}

void CaptureWidget::childLeave()
{
    m_previewEnabled = true;
    updateToolMousePreview(activeButtonTool());
}

void CaptureWidget::copyScreenshot()
{
    m_captureDone = true;
    if (m_activeTool != nullptr) {
        processPixmapWithTool(&m_context.screenshot, m_activeTool);
    }

    auto req = Controller::getInstance()->requests().find(m_id);
    req->addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);

    close();
}

void CaptureWidget::saveScreenshot()
{
#if defined(Q_OS_MACOS)
    showNormal();
#endif
    m_captureDone = true;
    if (m_activeTool != nullptr) {
        processPixmapWithTool(&m_context.screenshot, m_activeTool);
    }
    hide();
    if (m_context.savePath.isEmpty()) {
        ScreenshotSaver(m_id).saveToFilesystemGUI(pixmap());
    } else {
        ScreenshotSaver(m_id).saveToFilesystem(pixmap(), m_context.savePath);
    }
    close();
}

void CaptureWidget::setCaptureToolObjects(
  const CaptureToolObjects& captureToolObjects)
{
    // Used for undo/redo
    m_captureToolObjects = captureToolObjects;
    drawToolsData();
    updateLayersPanel();
    drawObjectSelection();
}

void CaptureWidget::undo()
{
    if (m_activeTool &&
        (m_activeTool->isChanged() || m_activeTool->editMode())) {
        // Remove selection on undo, at the same time commit current tool will
        // be called
        m_panel->setActiveLayer(-1);
    }

    // drawToolsData is called twice to update both previous and new regions
    // FIXME this is a temporary workaround
    drawToolsData();
    m_undoStack.undo();
    drawToolsData();
    updateLayersPanel();
}

void CaptureWidget::redo()
{
    // drawToolsData is called twice to update both previous and new regions
    // FIXME this is a temporary workaround
    drawToolsData();
    m_undoStack.redo();
    drawToolsData();
    update();
    updateLayersPanel();
}

QRect CaptureWidget::extendedSelection() const
{
    if (!m_selection->isVisible()) {
        return QRect();
    }
    QRect r = m_selection->geometry();
    return extendedRect(r);
}

QRect CaptureWidget::extendedRect(const QRect& r) const
{
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();
    return QRect(r.left() * devicePixelRatio,
                 r.top() * devicePixelRatio,
                 r.width() * devicePixelRatio,
                 r.height() * devicePixelRatio);
}

QRect CaptureWidget::paddedUpdateRect(const QRect& r) const
{
    if (r.isNull()) {
        return r;
    } else {
        return r + QMargins(20, 20, 20, 20);
    }
}

void CaptureWidget::drawConfigErrorMessage(QPainter* painter)
{
    QString msg;
    if (m_configError) {
        msg = ConfigHandler().errorMessage();
    } else if (m_configErrorResolved) {
        msg = tr("Configuration error resolved. Launch `flameshot "
                 "gui` again to apply it.");
    }

    auto textRect = painter->fontMetrics().boundingRect(msg);
    int w = textRect.width(), h = textRect.height();
    textRect = { size().width() - w, size().height() - h, w + 100, h + 100 };
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();

    if (!textRect.contains(QCursor::pos(currentScreen))) {
        QColor textColor(Qt::white);
        painter->setPen(textColor);
        painter->drawText(textRect, msg);
    }
}

void CaptureWidget::drawInactiveRegion(QPainter* painter)
{
    QColor overlayColor(0, 0, 0, m_opacity);
    painter->setBrush(overlayColor);
    QRect r;
    if (m_selection->isVisible()) {
        r = m_selection->geometry().normalized();
    }
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter->setClipRegion(grey);
    painter->drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
}
