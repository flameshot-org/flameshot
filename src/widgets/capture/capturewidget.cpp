// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#include "capturewidget.h"
#include "abstractlogger.h"
#include "copytool.h"
#include "src/config/cacheutils.h"
#include "src/config/generalconf.h"
#include "src/core/flameshot.h"
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

#if !defined(DISABLE_UPDATE_CHECKER)
#include "src/widgets/updatenotificationwidget.h"
#endif

#define MOUSE_DISTANCE_TO_START_MOVING 3

// CaptureWidget is the main component used to capture the screen. It contains
// an area of selection with its respective buttons.

// enableSaveWindow

CaptureWidget::CaptureWidget(const CaptureRequest& req,
                             bool fullScreen,
                             QWidget* parent)
  : QWidget(parent)
  , m_toolSizeByKeyboard(0)
  , m_mouseIsClicked(false)
  , m_captureDone(false)
  , m_previewEnabled(true)
  , m_adjustmentButtonPressed(false)
  , m_configError(false)
  , m_configErrorResolved(false)
#if !defined(DISABLE_UPDATE_CHECKER)
  , m_updateNotificationWidget(nullptr)
#endif
  , m_lastMouseWheel(0)
  , m_activeButton(nullptr)
  , m_activeTool(nullptr)
  , m_activeToolIsMoved(false)
  , m_toolWidget(nullptr)
  , m_panel(nullptr)
  , m_sidePanel(nullptr)
  , m_colorPicker(nullptr)
  , m_selection(nullptr)
  , m_magnifier(nullptr)
  , m_xywhDisplay(false)
  , m_existingObjectIsChanged(false)
  , m_startMove(false)

{
    m_undoStack.setUndoLimit(ConfigHandler().undoLimit());
    m_context.circleCount = 1;

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
    connect(&m_xywhTimer, &QTimer::timeout, this, &CaptureWidget::xywhTick);
    // else xywhTick keeps triggering when not needed
    m_xywhTimer.setSingleShot(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    m_opacity = m_config.contrastOpacity();
    m_uiColor = m_config.uiColor();
    m_contrastUiColor = m_config.contrastUiColor();
    setMouseTracking(true);
    initContext(fullScreen, req);
#if (defined(Q_OS_WIN) || defined(Q_OS_MACOS))
    // Top left of the whole set of screens
    QPoint topLeft(0, 0);
#endif
    if (fullScreen) {
        // Grab Screenshot
        bool ok = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(ok);
        if (!ok) {
            AbstractLogger::error() << tr("Unable to capture screen");
            this->close();
        }
        m_context.origScreenshot = m_context.screenshot;

#if defined(Q_OS_WIN)
// Call cmake with -DFLAMESHOT_DEBUG_CAPTURE=ON to enable easier debugging
#if !defined(FLAMESHOT_DEBUG_CAPTURE)
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                       Qt::SubWindow // Hides the taskbar icon
        );
#endif

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
// Call cmake with -DFLAMESHOT_DEBUG_CAPTURE=ON to enable easier debugging
#if !defined(FLAMESHOT_DEBUG_CAPTURE)
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint |
                       Qt::FramelessWindowHint | Qt::Tool);
        resize(pixmap().size());
#endif
#endif
    }
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

    m_buttonHandler = new ButtonHandler(this);
    m_buttonHandler->updateScreenRegions(areas);
    m_buttonHandler->hide();

    initButtons();
    initSelection(); // button handler must be initialized before
    initShortcuts(); // must be called after initSelection
    // init magnify
    if (m_config.showMagnifier()) {
        m_magnifier = new MagnifierWidget(
          m_context.screenshot, m_uiColor, m_config.squareMagnifier(), this);
    }

    // Init color picker
    m_colorPicker = new ColorPicker(this);
    connect(m_colorPicker,
            &ColorPicker::colorSelected,
            this,
            [this](const QColor& c) {
                m_context.mousePos = mapFromGlobal(QCursor::pos());
                setDrawColor(c);
            });
    m_colorPicker->hide();

    // Init tool size sigslots
    connect(this,
            &CaptureWidget::toolSizeChanged,
            this,
            &CaptureWidget::onToolSizeChanged);

    // Init notification widget
    m_notifierBox = new NotifierBox(this);
    m_notifierBox->hide();
    connect(m_notifierBox, &NotifierBox::hidden, this, [this]() {
        // Show cursor if it was hidden while adjusting tool size
        updateCursor();
        m_toolSizeByKeyboard = 0;
        onToolSizeChanged(m_context.toolSize);
        onToolSizeSettled(m_context.toolSize);
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
        initHelpMessage();
        OverlayMessage::push(m_helpMessage);
    }

    updateCursor();
}

CaptureWidget::~CaptureWidget()
{
#if defined(Q_OS_MACOS)
    for (QWidget* widget : qApp->topLevelWidgets()) {
        QString className(widget->metaObject()->className());
        if (0 ==
            className.compare(CaptureWidget::staticMetaObject.className())) {
            widget->showNormal();
            widget->hide();
            break;
        }
    }
#endif
    if (m_captureDone) {
        auto lastRegion = m_selection->geometry();
        setLastRegion(lastRegion);
        QRect geometry(m_context.selection);
        geometry.setTopLeft(geometry.topLeft() + m_context.widgetOffset);
        Flameshot::instance()->exportCapture(
          pixmap(), geometry, m_context.request);
    } else {
        emit Flameshot::instance()->captureFailed();
    }
}

void CaptureWidget::initButtons()
{
    auto allButtonTypes = CaptureToolButton::getIterableButtonTypes();
    auto visibleButtonTypes = m_config.buttons();
    if ((m_context.request.tasks() == CaptureRequest::NO_TASK) ||
        (m_context.request.tasks() == CaptureRequest::PRINT_GEOMETRY)) {
        allButtonTypes.removeOne(CaptureTool::TYPE_ACCEPT);
        visibleButtonTypes.removeOne(CaptureTool::TYPE_ACCEPT);
    } else {
        // Remove irrelevant buttons from both lists
        for (auto* buttonList : { &allButtonTypes, &visibleButtonTypes }) {
            buttonList->removeOne(CaptureTool::TYPE_SAVE);
            buttonList->removeOne(CaptureTool::TYPE_COPY);
            buttonList->removeOne(CaptureTool::TYPE_IMAGEUPLOADER);
            buttonList->removeOne(CaptureTool::TYPE_OPEN_APP);
            buttonList->removeOne(CaptureTool::TYPE_PIN);
        }
    }
    QVector<CaptureToolButton*> vectorButtons;

    // Add all buttons but hide those that were disabled in the Interface config
    // This will allow keyboard shortcuts for those buttons to work
    for (CaptureTool::Type t : allButtonTypes) {
        auto* b = new CaptureToolButton(t, this);
        if (t == CaptureTool::TYPE_SELECTIONINDICATOR) {
            m_sizeIndButton = b;
        }
        b->setColor(m_uiColor);
        b->hide();
        // must be enabled for SelectionWidget's eventFilter to work correctly
        b->setAttribute(Qt::WA_NoMousePropagation);
        makeChild(b);

        switch (t) {
            case CaptureTool::TYPE_UNDO:
            case CaptureTool::TYPE_REDO:
                // nothing to do, just skip non-dynamic buttons with existing
                // hard coded slots
                break;
            default:
                // Set shortcuts for a tool
                QString shortcut =
                  ConfigHandler().shortcut(QVariant::fromValue(t).toString());
                if (!shortcut.isNull()) {
                    auto shortcuts = newShortcut(shortcut, this, nullptr);
                    for (auto* sc : shortcuts) {
                        connect(sc, &QShortcut::activated, this, [=]() {
                            setState(b);
                        });
                    }
                }
                break;
        }

        m_tools[t] = b->tool();

        connect(b->tool(),
                &CaptureTool::requestAction,
                this,
                &CaptureWidget::handleToolSignal);

        if (visibleButtonTypes.contains(t)) {
            connect(b,
                    &CaptureToolButton::pressedButtonLeftClick,
                    this,
                    &CaptureWidget::handleButtonLeftClick);

            if (b->tool()->isSelectable()) {
                connect(b,
                        &CaptureToolButton::pressedButtonRightClick,
                        this,
                        &CaptureWidget::handleButtonRightClick);
            }

            vectorButtons << b;
        }
    }
    m_buttonHandler->setButtons(vectorButtons);
}

void CaptureWidget::handleButtonRightClick(CaptureToolButton* b)
{
    if (!b) {
        return;
    }

    // if button already selected, do not deselect it on right click
    if (!m_activeButton || m_activeButton != b) {
        setState(b);
    }
    if (!m_panel->isVisible()) {
        m_panel->show();
    }
}

void CaptureWidget::handleButtonLeftClick(CaptureToolButton* b)
{
    if (!b) {
        return;
    }
    setState(b);
}

void CaptureWidget::xywhTick()
{
    m_xywhDisplay = false;
    update();
}

void CaptureWidget::onDisplayGridChanged(bool display)
{
    m_displayGrid = display;
    repaint();
}

void CaptureWidget::onGridSizeChanged(int size)
{
    m_gridSize = size;
    repaint();
}

void CaptureWidget::showxywh()
{
    m_xywhDisplay = true;
    update();
    int timeout = m_config.showSelectionGeometryHideTime();
    if (timeout != 0) {
        m_xywhTimer.start(timeout);
    }
}

void CaptureWidget::initHelpMessage()
{
    QList<QPair<QString, QString>> keyMap;
    keyMap << QPair(tr("Mouse"), tr("Select screenshot area"));
    using CT = CaptureTool;
    for (auto toolType : { CT::TYPE_ACCEPT, CT::TYPE_SAVE, CT::TYPE_COPY }) {
        if (!m_tools.contains(toolType)) {
            continue;
        }
        auto* tool = m_tools[toolType];
        QString shortcut =
          ConfigHandler().shortcut(QVariant::fromValue(toolType).toString());
        shortcut.replace("Return", "Enter");
        if (!shortcut.isEmpty()) {
            keyMap << QPair(shortcut, tool->description());
        }
    }
    keyMap << QPair(tr("Mouse Wheel"), tr("Change tool size"));
    keyMap << QPair(tr("Right Click"), tr("Show color picker"));
    keyMap << QPair(ConfigHandler().shortcut("TYPE_TOGGLE_PANEL"),
                    tr("Open side panel"));
    keyMap << QPair(tr("Esc"), tr("Exit"));

    m_helpMessage = OverlayMessage::compileFromKeyMap(keyMap);
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
        processPixmapWithTool(&m_context.screenshot, m_activeTool);
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
        m_toolWidget->hide();
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
        m_toolWidget->hide();
        delete m_toolWidget;
        m_toolWidget = nullptr;
    }
}

void CaptureWidget::uncheckActiveTool()
{
    // uncheck active tool
    m_panel->setToolWidget(nullptr);
    m_activeButton->setColor(m_uiColor);
    updateTool(activeButtonTool());
    m_activeButton = nullptr;
    releaseActiveTool();
    updateSelectionState();
    updateCursor();
}

void CaptureWidget::paintEvent(QPaintEvent* paintEvent)
{
    Q_UNUSED(paintEvent)
    QPainter painter(this);
    GeneralConf::xywh_position position =
      static_cast<GeneralConf::xywh_position>(m_config.showSelectionGeometry());
    /* QPainter::save and restore is somewhat costly so we try to guess
       if we need to do it here. What that means is that if you add
       anything to the paintEvent and want to save/restore you should
       add a test to the below if statement -- also if you change
       any of the conditions that current trigger it you'll need to change here,
       too
    */
    bool save = false;
    if (m_xywhDisplay ||                           // clause 1: xywh display
        m_displayGrid ||                           // clause 2: display grid
        (m_activeTool && m_mouseIsClicked) ||      // clause 3: tool/click
        (m_previewEnabled && activeButtonTool() && // clause 4: mouse preview
         m_activeButton->tool()->showMousePreview())) {
        painter.save();
        save = true;
    }
    painter.drawPixmap(0, 0, m_context.screenshot);
    if (m_selection && m_xywhDisplay) {
        const QRect& selection = m_selection->geometry().normalized();
        const qreal scale = m_context.screenshot.devicePixelRatio();
        QRect xybox;
        QFontMetrics fm = painter.fontMetrics();

        QString xy = QString("%1x%2+%3+%4")
                       .arg(static_cast<int>(selection.width() * scale))
                       .arg(static_cast<int>(selection.height() * scale))
                       .arg(static_cast<int>(selection.left() * scale))
                       .arg(static_cast<int>(selection.top() * scale));

        xybox = fm.boundingRect(xy);
        // the small numbers here are just margins so the text doesn't
        // smack right up to the box; they aren't critical and the box
        // size itself is tied to the font metrics
        xybox.adjust(0, 0, 10, 12);
        // in anticipation of making the position adjustable
        int x0, y0;
        // Move these to header

        switch (position) {
            case GeneralConf::xywh_top_left:
                x0 = selection.left();
                y0 = selection.top();
                break;
            case GeneralConf::xywh_bottom_left:
                x0 = selection.left();
                y0 = selection.bottom() - xybox.height();
                break;
            case GeneralConf::xywh_top_right:
                x0 = selection.right() - xybox.width();
                y0 = selection.top();
                break;
            case GeneralConf::xywh_bottom_right:
                x0 = selection.right() - xybox.width();
                y0 = selection.bottom() - xybox.height();
                break;
            case GeneralConf::xywh_center:
            default:
                x0 = selection.left() + (selection.width() - xybox.width()) / 2;
                y0 =
                  selection.top() + (selection.height() - xybox.height()) / 2;
        }

        QColor uicolor = ConfigHandler().uiColor();
        uicolor.setAlpha(200);
        painter.fillRect(
          x0, y0, xybox.width(), xybox.height(), QBrush(uicolor));
        painter.setPen(ColorUtils::colorIsDark(uicolor) ? Qt::white
                                                        : Qt::black);
        painter.drawText(x0,
                         y0,
                         xybox.width(),
                         xybox.height(),
                         Qt::AlignVCenter | Qt::AlignHCenter,
                         xy);
    }

    if (m_displayGrid) {
        QColor uicolor = ConfigHandler().uiColor();
        uicolor.setAlpha(100);
        painter.setPen(uicolor);
        painter.setBrush(QBrush(uicolor));

        auto topLeft = mapToGlobal(m_context.selection.topLeft());
        topLeft.rx() -= topLeft.x() % m_gridSize;
        topLeft.ry() -= topLeft.y() % m_gridSize;
        topLeft = mapFromGlobal(topLeft);

        const auto scale{ m_context.screenshot.devicePixelRatio() };
        const auto step{ m_gridSize * scale };
        const auto radius{ 1 * scale };

        for (int y = topLeft.y(); y < m_context.selection.bottom(); y += step) {
            for (int x = topLeft.x(); x < m_context.selection.right();
                 x += step) {
                painter.drawEllipse(x, y, radius, radius);
            }
        }
    }

    if (m_activeTool && m_mouseIsClicked) {
        m_activeTool->process(painter, m_context.screenshot);
    } else if (m_previewEnabled && activeButtonTool() &&
               m_activeButton->tool()->showMousePreview()) {
        m_activeButton->tool()->paintMousePreview(painter, m_context);
    }
    if (save)
        painter.restore();
    // draw inactive region
    drawInactiveRegion(&painter);

    if (!isActiveWindow()) {
        drawErrorMessage(
          tr("Flameshot has lost focus. Keyboard shortcuts won't "
             "work until you click somewhere."),
          &painter);
    } else if (m_configError) {
        drawErrorMessage(ConfigHandler().errorMessage(), &painter);
    } else if (m_configErrorResolved) {
        drawErrorMessage(tr("Configuration error resolved. Launch `flameshot "
                            "gui` again to apply it."),
                         &painter);
    }
}

void CaptureWidget::showColorPicker(const QPoint& pos)
{
    // Try to select new object if current pos out of active object
    auto toolItem = activeToolObject();
    if (!toolItem || (toolItem && !toolItem->boundingRect().contains(pos))) {
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
                &CaptureWidget::toolSizeChanged,
                m_activeTool,
                &CaptureTool::onSizeChanged);
        connect(m_activeTool,
                &CaptureTool::requestAction,
                this,
                &CaptureWidget::handleToolSignal);

        m_context.mousePos = m_displayGrid ? snapToGrid(pos) : pos;
        m_activeTool->drawStart(m_context);
        // TODO this is the wrong place to do this

        if (m_activeTool->type() == CaptureTool::TYPE_CIRCLECOUNT) {
            m_activeTool->setCount(m_context.circleCount++);
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
            int oldToolSize = m_context.toolSize;
            m_panel->setActiveLayer(activeLayerIndex);
            drawObjectSelection();
            if (oldToolSize != m_context.toolSize) {
                emit toolSizeChanged(m_context.toolSize);
            }
        }
    }
    return activeLayerIndex;
}

void CaptureWidget::mousePressEvent(QMouseEvent* e)
{
    activateWindow();
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
        if ((event->button() == Qt::LeftButton) &&
            (m_config.copyOnDoubleClick())) {
            CopyTool copyTool;
            connect(&copyTool,
                    &CopyTool::requestAction,
                    this,
                    &CaptureWidget::handleToolSignal);
            copyTool.pressed(m_context);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }
}

void CaptureWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_magnifier) {
        if (!m_activeButton) {
            m_magnifier->show();
            m_magnifier->update();
        } else {
            m_magnifier->hide();
        }
    }

    m_context.mousePos = e->pos();
    if (e->buttons() != Qt::LeftButton) {
        updateTool(activeButtonTool());
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
            m_activeTool->drawMove(m_displayGrid ? snapToGrid(e->pos())
                                                 : e->pos());
        }
        // update drawing object
        updateTool(m_activeTool);
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

/**
 * Was updateThickness.
 * - Update tool mouse preview
 * - Show notifier box displaying the new thickness
 * - Update selected object thickness
 */
void CaptureWidget::setToolSize(int size)
{
    int oldSize = m_context.toolSize;
    m_context.toolSize = qBound(1, size, maxToolSize);
    updateTool(activeButtonTool());

    QPoint topLeft =
      QGuiAppCurrentScreen().currentScreen()->geometry().topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_context.toolSize));

    if (m_context.toolSize != oldSize) {
        emit toolSizeChanged(m_context.toolSize);
    }
}

void CaptureWidget::keyPressEvent(QKeyEvent* e)
{
    // If the key is a digit, change the tool size
    bool ok;
    int digit = e->text().toInt(&ok);
    if (ok && ((e->modifiers() == Qt::NoModifier) ||
               e->modifiers() == Qt::KeypadModifier)) { // digit received
        m_toolSizeByKeyboard = 10 * m_toolSizeByKeyboard + digit;
        setToolSize(m_toolSizeByKeyboard);
        if (m_context.toolSize != m_toolSizeByKeyboard) {
            // The tool size was out of range and was clipped by setToolSize
            m_toolSizeByKeyboard = 0;
        }
    } else {
        m_toolSizeByKeyboard = 0;
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
    int toolSizeOffset = 0;
    if (e->angleDelta().y() >= 60) {
        // mouse scroll (wheel) increment
        toolSizeOffset = 1;
    } else if (e->angleDelta().y() <= -60) {
        // mouse scroll (wheel) decrement
        toolSizeOffset = -1;
    } else {
        // touchpad scroll
        qint64 current = QDateTime::currentMSecsSinceEpoch();
        if ((current - m_lastMouseWheel) > 200) {
            if (e->angleDelta().y() > 0) {
                toolSizeOffset = 1;
            } else if (e->angleDelta().y() < 0) {
                toolSizeOffset = -1;
            }
            m_lastMouseWheel = current;
        } else {
            return;
        }
    }

    setToolSize(m_context.toolSize + toolSizeOffset);
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

void CaptureWidget::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::ActivationChange) {
        QPoint bottomRight = rect().bottomRight();
        // Update the message in the bottom right corner. A rough estimate is
        // used for the update rect
        update(QRect(bottomRight - QPoint(1000, 200), bottomRight));
    }
}

void CaptureWidget::initContext(bool fullscreen, const CaptureRequest& req)
{
    m_context.color = m_config.drawColor();
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    m_context.mousePos = mapFromGlobal(QCursor::pos());
    m_context.toolSize = m_config.drawThickness();
    m_context.fullscreen = fullscreen;

    // initialize m_context.request
    m_context.request = req;
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
    connect(m_panel,
            &UtilityPanel::moveUpClicked,
            this,
            &CaptureWidget::onMoveCaptureToolUp);
    connect(m_panel,
            &UtilityPanel::moveDownClicked,
            this,
            &CaptureWidget::onMoveCaptureToolDown);

    m_sidePanel = new SidePanelWidget(&m_context.screenshot, this);
    connect(m_sidePanel,
            &SidePanelWidget::colorChanged,
            this,
            &CaptureWidget::setDrawColor);
    connect(m_sidePanel,
            &SidePanelWidget::toolSizeChanged,
            this,
            &CaptureWidget::onToolSizeChanged);
    connect(this,
            &CaptureWidget::colorChanged,
            m_sidePanel,
            &SidePanelWidget::onColorChanged);
    connect(this,
            &CaptureWidget::toolSizeChanged,
            m_sidePanel,
            &SidePanelWidget::onToolSizeChanged);
    connect(m_sidePanel,
            &SidePanelWidget::togglePanel,
            m_panel,
            &UtilityPanel::toggle);
    connect(m_sidePanel,
            &SidePanelWidget::displayGridChanged,
            this,
            &CaptureWidget::onDisplayGridChanged);
    connect(m_sidePanel,
            &SidePanelWidget::gridSizeChanged,
            this,
            &CaptureWidget::onGridSizeChanged);
    // TODO replace with a CaptureWidget signal
    emit m_sidePanel->colorChanged(m_context.color);
    emit toolSizeChanged(m_context.toolSize);
    m_panel->pushWidget(m_sidePanel);

    // Fill undo/redo/history list widget
    m_panel->fillCaptureTools(m_captureToolObjects.captureToolObjects());
}

#if !defined(DISABLE_UPDATE_CHECKER)
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
#endif

void CaptureWidget::initSelection()
{
    // Be mindful of the order of statements, so that slots are called properly
    m_selection = new SelectionWidget(m_uiColor, this);
    QRect initialSelection = m_context.request.initialSelection();
    connect(m_selection, &SelectionWidget::geometryChanged, this, [this]() {
        QRect constrainedToCaptureArea =
          m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(constrainedToCaptureArea);

        m_buttonHandler->hide();
        updateCursor();
        updateSizeIndicator();
        OverlayMessage::pop();
    });
    connect(m_selection, &SelectionWidget::geometrySettled, this, [this]() {
        if (m_selection->isVisibleTo(this)) {
            auto& req = m_context.request;
            if (req.tasks() & CaptureRequest::ACCEPT_ON_SELECT) {
                req.removeTask(CaptureRequest::ACCEPT_ON_SELECT);
                m_captureDone = true;
                close();
            }
            m_buttonHandler->updatePosition(m_selection->geometry());
            m_buttonHandler->show();
        } else {
            m_buttonHandler->hide();
        }
    });
    connect(m_selection, &SelectionWidget::visibilityChanged, this, [this]() {
        if (!m_selection->isVisible() && !m_helpMessage.isEmpty()) {
            OverlayMessage::push(m_helpMessage);
        }
    });
    if (!initialSelection.isNull()) {
        const qreal scale = m_context.screenshot.devicePixelRatio();
        initialSelection.moveTopLeft(initialSelection.topLeft() -
                                     mapToGlobal({}));
        initialSelection.setTop(initialSelection.top() / scale);
        initialSelection.setBottom(initialSelection.bottom() / scale);
        initialSelection.setLeft(initialSelection.left() / scale);
        initialSelection.setRight(initialSelection.right() / scale);
    }
    m_selection->setGeometry(initialSelection);
    m_selection->setVisible(!initialSelection.isNull());
    if (!initialSelection.isNull()) {
        m_context.selection = extendedRect(m_selection->geometry());
        emit m_selection->geometrySettled();
    }
}

void CaptureWidget::setState(CaptureToolButton* b)
{
    if (!b) {
        return;
    }

    commitCurrentTool();
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

    if (b->tool()->isSelectable()) {
        if (m_activeButton != b) {
            if (m_activeButton) {
                m_activeButton->setColor(m_uiColor);
            }
            m_activeButton = b;
            m_activeButton->setColor(m_contrastUiColor);
            m_panel->setActiveLayer(-1);
            m_panel->setToolWidget(b->tool()->configurationWidget());
        } else if (m_activeButton) {
            m_panel->clearToolWidget();
            m_activeButton->setColor(m_uiColor);
            m_activeButton = nullptr;
        }
        m_context.toolSize = ConfigHandler().toolSize(activeButtonToolType());
        emit toolSizeChanged(m_context.toolSize);
        updateCursor();
        updateSelectionState();
        updateTool(b->tool());
    }
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
        case CaptureTool::REQ_CAPTURE_DONE_OK:
            m_captureDone = true;
            break;
        case CaptureTool::REQ_CLEAR_SELECTION:
            if (m_panel->activeLayerIndex() >= 0) {
                m_panel->setActiveLayer(-1);
                drawToolsData(false);
            }
            break;
        case CaptureTool::REQ_ADD_CHILD_WIDGET:
            if (!m_activeTool) {
                break;
            }
            if (m_toolWidget) {
                m_toolWidget->hide();
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
                Flameshot::instance()->setExternalWidget(true);
            }
            break;
        case CaptureTool::REQ_INCREASE_TOOL_SIZE:
            setToolSize(m_context.toolSize + 1);
            break;
        case CaptureTool::REQ_DECREASE_TOOL_SIZE:
            setToolSize(m_context.toolSize - 1);
            break;
        default:
            break;
    }
}

/**
 * Was setDrawThickness
 * - Update config options
 * - Update tool object thickness
 */
void CaptureWidget::onToolSizeChanged(int t)
{

    m_context.toolSize = t;
    CaptureTool* tool = activeButtonTool();
    if (tool && tool->showMousePreview()) {
        setCursor(Qt::BlankCursor);
        tool->onSizeChanged(t);
    }

    // update tool size of object being drawn
    if (m_activeTool != nullptr) {
        updateTool(m_activeTool);
    }

    // update tool size of selected object
    auto toolItem = activeToolObject();
    if (toolItem) {
        // Change thickness
        toolItem->onSizeChanged(t);
        if (!m_existingObjectIsChanged) {
            m_captureToolObjectsBackup = m_captureToolObjects;
            m_existingObjectIsChanged = true;
        }
        drawToolsData();
        updateTool(toolItem);
    }

    // Force a repaint to prevent artifacting
    this->repaint();
}

void CaptureWidget::onToolSizeSettled(int size)
{
    m_config.setToolSize(activeButtonToolType(), size);
}

void CaptureWidget::setDrawColor(const QColor& c)
{
    m_context.color = c;
    if (m_context.color.isValid()) {
        ConfigHandler().setDrawColor(m_context.color);
        emit colorChanged(c);
        // Update mouse preview
        updateTool(activeButtonTool());

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

void CaptureWidget::onMoveCaptureToolUp(int captureToolIndex)
{
    m_captureToolObjectsBackup = m_captureToolObjects;
    pushObjectsStateToUndoStack();
    auto tool = m_captureToolObjects.at(captureToolIndex);
    m_captureToolObjects.removeAt(captureToolIndex);
    m_captureToolObjects.insert(captureToolIndex - 1, tool);
    updateLayersPanel();
}

void CaptureWidget::onMoveCaptureToolDown(int captureToolIndex)
{
    m_captureToolObjectsBackup = m_captureToolObjects;
    pushObjectsStateToUndoStack();
    auto tool = m_captureToolObjects.at(captureToolIndex);
    m_captureToolObjects.removeAt(captureToolIndex);
    m_captureToolObjects.insert(captureToolIndex + 1, tool);
    updateLayersPanel();
}

void CaptureWidget::selectAll()
{
    m_selection->show();
    m_selection->setGeometry(rect());
    emit m_selection->geometrySettled();
    m_buttonHandler->show();
    updateSelectionState();
}

void CaptureWidget::removeToolObject(int index)
{
    --index;
    if (index >= 0 && index < m_captureToolObjects.size()) {
        // in case this tool is circle counter
        const CaptureTool::Type currentToolType =
          m_captureToolObjects.at(index)->type();
        m_captureToolObjectsBackup = m_captureToolObjects;
        update(
          paddedUpdateRect(m_captureToolObjects.at(index)->boundingRect()));
        if (currentToolType == CaptureTool::TYPE_CIRCLECOUNT) {
            int removedCircleCount = m_captureToolObjects.at(index)->count();
            --m_context.circleCount;
            // Decrement circle counter numbers starting from deleted circle
            for (int cnt = 0; cnt < m_captureToolObjects.size(); cnt++) {
                auto toolItem = m_captureToolObjects.at(cnt);
                if (toolItem->type() != CaptureTool::TYPE_CIRCLECOUNT) {
                    continue;
                }
                auto circleTool = m_captureToolObjects.at(cnt);
                if (circleTool->count() >= removedCircleCount) {
                    circleTool->setCount(circleTool->count() - 1);
                }
            }
        }
        m_captureToolObjects.removeAt(index);
        pushObjectsStateToUndoStack();
        drawToolsData();
        updateLayersPanel();
    }
}

void CaptureWidget::initShortcuts()
{
    newShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_UNDO")), this, SLOT(undo()));

    newShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_REDO")), this, SLOT(redo()));

    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_TOGGLE_PANEL")),
                this,
                SLOT(togglePanel()));

    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_LEFT")),
                m_selection,
                SLOT(resizeLeft()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_RIGHT")),
                m_selection,
                SLOT(resizeRight()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_UP")),
                m_selection,
                SLOT(resizeUp()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_RESIZE_DOWN")),
                m_selection,
                SLOT(resizeDown()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SYM_RESIZE_LEFT")),
                m_selection,
                SLOT(symResizeLeft()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SYM_RESIZE_RIGHT")),
                m_selection,
                SLOT(symResizeRight()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SYM_RESIZE_UP")),
                m_selection,
                SLOT(symResizeUp()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SYM_RESIZE_DOWN")),
                m_selection,
                SLOT(symResizeDown()));

    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_LEFT")),
                m_selection,
                SLOT(moveLeft()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_RIGHT")),
                m_selection,
                SLOT(moveRight()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_UP")),
                m_selection,
                SLOT(moveUp()));
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_MOVE_DOWN")),
                m_selection,
                SLOT(moveDown()));

    newShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_DELETE_CURRENT_TOOL")),
      this,
      SLOT(deleteCurrentTool()));

    newShortcut(
      QKeySequence(ConfigHandler().shortcut("TYPE_COMMIT_CURRENT_TOOL")),
      this,
      SLOT(commitCurrentTool()));

    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_SELECT_ALL")),
                this,
                SLOT(selectAll()));

    newShortcut(Qt::Key_Escape, this, SLOT(deleteToolWidgetOrClose()));
}

void CaptureWidget::deleteCurrentTool()
{
    int oldToolSize = m_context.toolSize;
    m_panel->slotButtonDelete(true);
    drawObjectSelection();
    if (oldToolSize != m_context.toolSize) {
        emit toolSizeChanged(m_context.toolSize);
    }
}

void CaptureWidget::updateSizeIndicator()
{
    if (m_config.showSelectionGeometry()) {
        showxywh();
    }
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

void CaptureWidget::updateTool(CaptureTool* tool)
{
    if (!tool || !tool->showMousePreview()) {
        return;
    }

    static QRect oldPreviewRect, oldToolObjectRect;

    QRect previewRect(tool->mousePreviewRect(m_context));
    previewRect += QMargins(previewRect.width(),
                            previewRect.height(),
                            previewRect.width(),
                            previewRect.height());

    QRect toolObjectRect = paddedUpdateRect(tool->boundingRect());

    // old rects are united with current rects to handle sudden mouse movement
    update(previewRect);
    update(toolObjectRect);
    update(oldPreviewRect);
    update(oldToolObjectRect);

    oldPreviewRect = previewRect;
    oldToolObjectRect = toolObjectRect;
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
                   &CaptureWidget::toolSizeChanged,
                   m_activeTool,
                   &CaptureTool::onSizeChanged);
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

void CaptureWidget::drawToolsData(bool drawSelection)
{
    // TODO refactor this for performance. The objects should not all be updated
    // at once every time
    QPixmap pixmapItem = m_context.origScreenshot;
    for (auto toolItem : m_captureToolObjects.captureToolObjects()) {
        processPixmapWithTool(&pixmapItem, toolItem);
        update(paddedUpdateRect(toolItem->boundingRect()));
    }

    m_context.screenshot = pixmapItem;
    if (drawSelection) {
        drawObjectSelection();
    }
}

void CaptureWidget::drawObjectSelection()
{
    auto toolItem = activeToolObject();
    if (toolItem && !toolItem->editMode()) {
        QPainter painter(&m_context.screenshot);
        toolItem->drawObjectSelection(painter);
        // TODO move this elsewhere
        if (m_context.toolSize != toolItem->size()) {
            m_context.toolSize = toolItem->size();
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

QPoint CaptureWidget::snapToGrid(const QPoint& point) const
{
    QPoint snapPoint = mapToGlobal(point);

    const auto scale{ m_context.screenshot.devicePixelRatio() };

    snapPoint.setX((qRound(snapPoint.x() / double(m_gridSize)) * m_gridSize) *
                   scale);
    snapPoint.setY((qRound(snapPoint.y() / double(m_gridSize)) * m_gridSize) *
                   scale);

    return mapFromGlobal(snapPoint);
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

void CaptureWidget::restoreCircleCountState()
{
    int largest = 0;
    for (int cnt = 0; cnt < m_captureToolObjects.size(); cnt++) {
        auto toolItem = m_captureToolObjects.at(cnt);
        if (toolItem->type() != CaptureTool::TYPE_CIRCLECOUNT) {
            continue;
        }
        if (toolItem->count() > largest) {
            largest = toolItem->count();
        }
    }
    m_context.circleCount = largest + 1;
}

/**
 * @brief Wrapper around `new QShortcut`, properly handling Enter/Return.
 */
QList<QShortcut*> CaptureWidget::newShortcut(const QKeySequence& key,
                                             QWidget* parent,
                                             const char* slot)
{
    QList<QShortcut*> shortcuts;
    QString strKey = key.toString();
    if (strKey.contains("Enter") || strKey.contains("Return")) {
        strKey.replace("Enter", "Return");
        shortcuts << new QShortcut(strKey, parent, slot);
        strKey.replace("Return", "Enter");
        shortcuts << new QShortcut(strKey, parent, slot);
    } else {
        shortcuts << new QShortcut(key, parent, slot);
    }
    return shortcuts;
}

void CaptureWidget::togglePanel()
{
    m_panel->toggle();
}

void CaptureWidget::childEnter()
{
    m_previewEnabled = false;
    updateTool(activeButtonTool());
}

void CaptureWidget::childLeave()
{
    m_previewEnabled = true;
    updateTool(activeButtonTool());
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

    restoreCircleCountState();
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

    restoreCircleCountState();
}

QRect CaptureWidget::extendedSelection() const
{
    if (m_selection == nullptr) {
        return {};
    }
    QRect r = m_selection->geometry();
    return extendedRect(r);
}

QRect CaptureWidget::extendedRect(const QRect& r) const
{
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();
    return { static_cast<int>(r.left() * devicePixelRatio),
             static_cast<int>(r.top() * devicePixelRatio),
             static_cast<int>(r.width() * devicePixelRatio),
             static_cast<int>(r.height() * devicePixelRatio) };
}

QRect CaptureWidget::paddedUpdateRect(const QRect& r) const
{
    if (r.isNull()) {
        return r;
    } else {
        return r + QMargins(20, 20, 20, 20);
    }
}

void CaptureWidget::drawErrorMessage(const QString& msg, QPainter* painter)
{
    auto textRect = painter->fontMetrics().boundingRect(msg);
    int w = textRect.width(), h = textRect.height();
    textRect = {
        size().width() - w - 10, size().height() - h - 5, w + 100, h + 100
    };
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
