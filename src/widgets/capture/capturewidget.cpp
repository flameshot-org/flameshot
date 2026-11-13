// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#include "capturewidget.h"
#include "config/cacheutils.h"
#include "config/generalconf.h"
#include "core/flameshot.h"
#include "core/qguiappcurrentscreen.h"
#include "tools/copy/copytool.h"
#include "utils/abstractlogger.h"
#include "utils/screengrabber.h"
#include "utils/screenshotsaver.h"
#include "widgets/capture/colorpicker.h"
#include "widgets/capture/hovereventfilter.h"
#include "widgets/capture/modificationcommand.h"
#include "widgets/capture/notifierbox.h"
#include "widgets/capture/overlaymessage.h"
#include "widgets/draggablewidgetmaker.h"
#include "widgets/orientablepushbutton.h"
#include "widgets/panel/sidepanelwidget.h"
#include "widgets/panel/utilitypanel.h"

#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QFontMetrics>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QTextEdit>
#include <QWindow>
#include <limits>
#include <utility>

#if !defined(DISABLE_UPDATE_CHECKER)
#include "widgets/updatenotificationwidget.h"
#endif

#define MOUSE_DISTANCE_TO_START_MOVING 3

auto const MOUSE_WHEEL_TRESHOLD = 60;

class ScreenOverlay : public QWidget
{
public:
    ScreenOverlay(CaptureWidget* controller,
                  QScreen* screen,
                  const QRect& logicalGeom,
                  const QPoint& canvasOffset)
      : QWidget(nullptr)
      , m_controller(controller)
      , m_screen(screen)
      , m_logicalGeom(logicalGeom)
      , m_canvasOffset(canvasOffset)
    {
        setAttribute(Qt::WA_QuitOnClose, false);
#if !defined(FLAMESHOT_DEBUG_CAPTURE)
        if (DesktopInfo().waylandDetected()) {
            setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                           Qt::NoDropShadowWindowHint |
                           Qt::WindowStaysOnTopHint);
        } else {
            setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                           Qt::NoDropShadowWindowHint |
                           Qt::WindowStaysOnTopHint);
        }
#endif
        setWindowState(Qt::WindowFullScreen);
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        syncWithScreen();
        if (m_screen) {
            QObject::connect(
              m_screen, &QScreen::geometryChanged, this, [this](const QRect&) {
                  if (m_screen) {
                      m_logicalGeom =
                        ScreenGrabber().screenLogicalGeometry(m_screen);
                      syncWithScreen();
                  }
              });
        }
    }

    QScreen* targetScreen() const { return m_screen; }
    QPoint canvasOffset() const { return m_canvasOffset; }
    QRect canvasRect() const { return QRect(m_canvasOffset, size()); }

    void syncWithScreen()
    {
        if (!m_screen) {
            return;
        }
        winId();
        if (QWindow* wh = windowHandle()) {
            if (wh->screen() != m_screen) {
                wh->setScreen(m_screen);
            }
            if (!m_screenConn) {
                m_screenConn = QObject::connect(
                  wh, &QWindow::screenChanged, this, [this](QScreen* s) {
                      if (m_screen && s != m_screen && windowHandle()) {
                          windowHandle()->setScreen(m_screen);
                          setFixedSize(m_logicalGeom.size());
                          setGeometry(m_logicalGeom);
                      }
                  });
            }
        }
        setFixedSize(m_logicalGeom.size());
        setGeometry(m_logicalGeom);
    }

protected:
    void paintEvent(QPaintEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->paintScreenOverlay(this, e);
        }
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        activateWindow();
        if (m_controller && !m_controller->m_closing) {
            m_controller->forwardOverlayMouseEvent(this, e);
        }
        e->accept();
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->forwardOverlayMouseEvent(this, e);
        }
        e->accept();
    }

    void mouseReleaseEvent(QMouseEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->forwardOverlayMouseEvent(this, e);
        }
        e->accept();
    }

    void mouseDoubleClickEvent(QMouseEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->forwardOverlayMouseEvent(this, e);
        }
        e->accept();
    }

    void wheelEvent(QWheelEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->wheelEvent(e);
        }
    }

    void keyPressEvent(QKeyEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->keyPressEvent(e);
        }
    }

    void keyReleaseEvent(QKeyEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            m_controller->keyReleaseEvent(e);
        }
    }

    void changeEvent(QEvent* e) override
    {
        QWidget::changeEvent(e);
        if (e->type() == QEvent::ActivationChange && m_controller &&
            !m_controller->m_closing) {
            m_controller->updateCanvas();
        }
    }

    void resizeEvent(QResizeEvent* e) override
    {
        QWidget::resizeEvent(e);
        if (m_controller && !m_controller->m_closing) {
            m_controller->onOverlayResized(this);
        }
    }

    void closeEvent(QCloseEvent* e) override
    {
        if (m_controller && !m_controller->m_closing) {
            e->ignore();
            m_controller->close();
            return;
        }
        QWidget::closeEvent(e);
    }

private:
    QPointer<CaptureWidget> m_controller;
    QPointer<QScreen> m_screen;
    QRect m_logicalGeom;
    QPoint m_canvasOffset;
    QMetaObject::Connection m_screenConn;
};

// CaptureWidget is the main component used to capture the screen. It contains
// an area of selection with its respective buttons.

// enableSaveWindow

CaptureWidget::CaptureWidget(const CaptureRequest& req,
                             bool fullScreen,
                             QWidget* parent)
  : QWidget(parent)
  , m_toolSizeByKeyboard(0)
  , m_mouseIsClicked(false)
  , m_newSelection(false)
  , m_movingSelection(false)
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
  , m_buttonHandler(nullptr)
  , m_panel(nullptr)
  , m_sidePanel(nullptr)
  , m_colorPicker(nullptr)
  , m_notifierBox(nullptr)
  , m_eventFilter(nullptr)
  , m_selection(nullptr)
  , m_magnifier(nullptr)
  , m_xywhDisplay(false)
  , m_existingObjectIsChanged(false)
  , m_startMove(false)
  , m_clipboardWorkaroundDone(false)

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

    ScreenGrabber grabber;
    QScreen* selectedScreen = nullptr;

#if (defined(Q_OS_WIN) || defined(Q_OS_MACOS))
    // Top left of the whole set of screens
    QPoint topLeft(0, 0);
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_multiMonitor = fullScreen && !req.hasSelectedMonitor() &&
                     m_config.captureAllMonitors() &&
                     QGuiApplication::screens().size() > 1;
#endif
    if (m_multiMonitor) {
        setAttribute(Qt::WA_DontShowOnScreen, true);
        bool ok = true;
        m_context.screenshot = grabber.grabFullDesktop(ok);
        m_desktopGeometry = grabber.logicalDesktopGeometry();
        if (!ok || m_context.screenshot.isNull() ||
            m_desktopGeometry.isEmpty()) {
            m_closing = true;
            this->close();
        } else {
            const qreal dpr = static_cast<qreal>(m_context.screenshot.width()) /
                              static_cast<qreal>(m_desktopGeometry.width());
            m_context.screenshot.setDevicePixelRatio(dpr > 0.0 ? dpr : 1.0);
            m_context.origScreenshot = m_context.screenshot;
            resize(m_desktopGeometry.size());
            m_context.widgetOffset = m_desktopGeometry.topLeft();

            const QList<QScreen*> screens = QGuiApplication::screens();
            for (QScreen* screen : screens) {
                const QRect screenGeom = grabber.screenLogicalGeometry(screen);
                const QPoint offset =
                  screenGeom.topLeft() - m_desktopGeometry.topLeft();
                m_overlays.append(
                  new ScreenOverlay(this, screen, screenGeom, offset));
            }
        }
    } else if (fullScreen) {
        bool ok = true;
        int preSelectedMonitor;
        if (req.hasSelectedMonitor()) {
            preSelectedMonitor = req.selectedMonitor();
        } else {
            preSelectedMonitor = -1;
        }
        m_context.screenshot =
          grabber.grabEntireDesktop(ok, preSelectedMonitor);
        if (!ok) {
            // Error already logged in ScreenGrabber
            m_closing = true;
            this->close();
        }
        m_context.origScreenshot = m_context.screenshot;

        selectedScreen = grabber.getSelectedScreen();

#if defined(Q_OS_WIN)
#if !defined(FLAMESHOT_DEBUG_CAPTURE)
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                       Qt::SubWindow // Hides the taskbar icon
        );
#endif
        // Position the window at the selected screen's position
        // (or the topLeft of all screens if no specific screen was selected)
        if (selectedScreen) {
            move(selectedScreen->geometry().topLeft());
        } else {
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
        }
        // On Windows, account for DPR when sizing the window
        QSize windowSize = pixmap().size();
        if (pixmap().devicePixelRatio() > 1.0) {
            windowSize = QSize(pixmap().width() / pixmap().devicePixelRatio(),
                               pixmap().height() / pixmap().devicePixelRatio());
        }
        resize(windowSize);

        if (selectedScreen != nullptr && windowHandle()) {
            windowHandle()->setScreen(selectedScreen);
        }
#elif defined(Q_OS_MACOS)
        if (!ConfigHandler().useNativeFullscreen()) {
            setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                           Qt::Tool);
        }
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        move(currentScreen->geometry().x(), currentScreen->geometry().y());
        resize(currentScreen->size());
// LINUX
#else
// Call cmake with -DFLAMESHOT_DEBUG_CAPTURE=ON to enable easier debugging
#if !defined(FLAMESHOT_DEBUG_CAPTURE)
        if (DesktopInfo().waylandDetected()) {
            setWindowFlags(Qt::BypassWindowManagerHint |
                           Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                           Qt::Tool);
        } else {
            // Note: Qt::BypassWindowManagerHint is removed to fix x11 gnome
            // crash. It's needed on Cosmic
            setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                           Qt::Tool);
        }
#endif

        // Always display on the selected screen (not spanning entire desktop)
        if (selectedScreen == nullptr) {
            selectedScreen = QGuiApplication::primaryScreen();
        }
        QRect screenGeom = selectedScreen->geometry();
        move(screenGeom.topLeft());
        resize(screenGeom.size());

        if (selectedScreen != nullptr && windowHandle()) {
            windowHandle()->setScreen(selectedScreen);
        }
        m_selectedScreen = selectedScreen;
#endif
    }

    QVector<QRect> areas;
    if (m_multiMonitor) {
        for (ScreenOverlay* overlay : m_overlays) {
            areas.append(overlay->canvasRect());
        }
        if (areas.isEmpty()) {
            areas.append(rect());
        }
    } else if (m_context.fullscreen) {
        // Always display on a single screen, normalized to (0, 0)
        QScreen* screenForAreas = selectedScreen;
        if (!screenForAreas) {
            screenForAreas = QGuiAppCurrentScreen().currentScreen();
        }
        if (!screenForAreas) {
            screenForAreas = QGuiApplication::primaryScreen();
        }
        QRect r = screenForAreas ? screenForAreas->geometry() : QRect();
        r.moveTo(0, 0);
        areas.append(r);
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
    if (m_config.showMagnifier() && !m_closing) {
        m_magnifier = new MagnifierWidget(
          m_context.screenshot, m_uiColor, m_config.squareMagnifier(), this);
    }

    // Init color picker
    m_colorPicker = new ColorPicker(this);
    m_colorPicker->setMultiMonitor(m_multiMonitor);
    connect(m_colorPicker, &ColorPicker::selectionChanged, this, [this]() {
        if (m_multiMonitor && m_colorPicker && m_colorPicker->isVisible()) {
            updateCanvas(colorPickerCanvasRect());
        }
    });
    // Init notification widget
    m_notifierBox = new NotifierBox(this);
    initPanel();

    // TODO: Make it more clear why this has moved. In Qt6 some timing related
    // to constructors / connect signals has changed so if initPanel is called
    // after the connect a SEGFAULT occurs
    connect(m_colorPicker,
            &ColorPicker::colorSelected,
            this,
            [this](const QColor& c) {
                if (!m_multiMonitor) {
                    m_context.mousePos = mapFromGlobal(QCursor::pos());
                }
                setDrawColor(c);
            });
    m_colorPicker->hide();

    // Init tool size sigslots
    connect(this,
            &CaptureWidget::toolSizeChanged,
            this,
            &CaptureWidget::onToolSizeChanged);

    m_notifierBox->hide();
    connect(m_notifierBox, &NotifierBox::hidden, this, [this]() {
        // Show cursor if it was hidden while adjusting tool size
        updateCursor();
        m_toolSizeByKeyboard = 0;
        onToolSizeChanged(m_context.toolSize);
    });

    m_config.checkAndHandleError();
    if (m_config.hasError()) {
        m_configError = true;
    }
    connect(
      ConfigHandler::getInstance(), &ConfigHandler::error, this, [=, this]() {
          m_configError = true;
          m_configErrorResolved = false;
          OverlayMessage::instance()->update();
      });
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::errorResolved,
            this,
            [=, this]() {
                m_configError = false;
                m_configErrorResolved = true;
                OverlayMessage::instance()->update();
            });

    // OverlayMessage is a child widget, so use widget-local coordinates
    // In fullscreen mode, use the normalized area; otherwise use widget rect
    if (m_multiMonitor) {
        if (ScreenOverlay* primary = primaryOverlay()) {
            OverlayMessage::init(primary, primary->rect());
        } else {
            OverlayMessage::init(this, rect());
        }
    } else {
        QRect overlayArea =
          m_context.fullscreen && !areas.isEmpty() ? areas.first() : rect();
        OverlayMessage::init(this, overlayArea);
    }

    if (m_config.showHelp()) {
        initHelpMessage();
        OverlayMessage::push(m_helpMessage);
    }

    initQuitPrompt();

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
        const qreal scale = m_context.screenshot.devicePixelRatio();
        lastRegion.setTop(lastRegion.top() * scale);
        lastRegion.setBottom(lastRegion.bottom() * scale);
        lastRegion.setLeft(lastRegion.left() * scale);
        lastRegion.setRight(lastRegion.right() * scale);
        setLastRegion(lastRegion);
        QRect geometry(m_context.selection);
        geometry.setTopLeft(geometry.topLeft() + m_context.widgetOffset);
        Flameshot::instance()->exportCapture(
          pixmap(), geometry, m_context.request);
    } else {
        emit Flameshot::instance()->captureFailed();
    }

    m_closing = true;
    if (m_notifierBox) {
        disconnect(m_notifierBox, nullptr, this, nullptr);
    }
    if (m_eventFilter) {
        disconnect(m_eventFilter, nullptr, this, nullptr);
    }
    const auto overlays = std::exchange(m_overlays, {});
    qDeleteAll(overlays);
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
#ifdef ENABLE_IMGUR
            buttonList->removeOne(CaptureTool::TYPE_IMAGEUPLOADER);
#endif
            buttonList->removeOne(CaptureTool::TYPE_OPEN_APP);
            buttonList->removeOne(CaptureTool::TYPE_PIN);
        }
    }
    QVector<CaptureToolButton*> vectorButtons;

    // Add all buttons but hide those that were disabled in the Interface config
    // This will allow keyboard shortcuts for those buttons to work
    for (CaptureTool::Type t : allButtonTypes) {
        auto* b = new CaptureToolButton(t, this);
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
                        connect(sc, &QShortcut::activated, this, [=, this]() {
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
    m_toolbarButtons = vectorButtons;
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
        if (m_multiMonitor) {
            ScreenOverlay* host = overlayForParent(b->parentWidget());
            if (!host) {
                host = overlayAtCanvasPos(m_context.mousePos);
            }
            syncPanelToOverlay(host);
        }
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
    updateCanvas();
}

void CaptureWidget::onDisplayGridChanged(bool display)
{
    m_displayGrid = display;
    repaintCanvas();
}

void CaptureWidget::onGridSizeChanged(int size)
{
    m_gridSize = size;
    repaintCanvas();
}

void CaptureWidget::startColorGrab()
{
    if (m_sidePanel) {
        m_sidePanel->startColorGrab();
    }
}

void CaptureWidget::showxywh()
{
    m_xywhDisplay = true;
    updateCanvas();
    int timeout = m_config.showSelectionGeometryHideTime();
    if (timeout != 0) {
        m_xywhTimer.start(timeout);
    }
}

void CaptureWidget::initHelpMessage()
{
    QList<QPair<QString, QString>> keyMap;
    keyMap << std::pair(tr("Mouse"), tr("Select screenshot area"));
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
            keyMap << std::pair(shortcut, tool->description());
        }
    }
    keyMap << std::pair(tr("Mouse Wheel"), tr("Change tool size"));
    keyMap << std::pair(tr("Right Click"), tr("Show color picker"));
    keyMap << std::pair(ConfigHandler().shortcut("TYPE_TOGGLE_PANEL"),
                        tr("Open side panel"));
    keyMap << std::pair(tr("Esc"), tr("Exit"));

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
        const QRect twRect = toolWidgetCanvasRect();
        processPixmapWithTool(&m_context.screenshot, m_activeTool);
        if (m_activeTool->isValid() && !m_activeTool->editMode() &&
            m_toolWidget) {
            pushToolToStack();
        }
        if (m_toolWidget) {
            m_toolWidget->update();
        }
        releaseActiveTool();
        if (m_multiMonitor && !twRect.isEmpty()) {
            updateCanvas(twRect);
        }
        return true;
    }
    return false;
}

void CaptureWidget::initQuitPrompt()
{
    m_quitPrompt = new QMessageBox;
    if (m_multiMonitor && primaryOverlay()) {
        m_quitPrompt->setParent(primaryOverlay(), Qt::Dialog);
        m_quitPrompt->installEventFilter(m_eventFilter);
    } else {
        makeChild(m_quitPrompt);
    }

    QString baseSheet = "QDialog { background-color: %1; }"
                        "QLabel, QCheckBox { color: %2 }"
                        "QPushButton { background-color: %1; color: %2 }";
    QColor text = ColorUtils::colorIsDark(m_uiColor) ? Qt::white : Qt::black;
    QString styleSheet = baseSheet.arg(m_uiColor.name(), text.name());

    m_quitPrompt->setStyleSheet(styleSheet);
    m_quitPrompt->setWindowTitle(tr("Quit Capture"));
    m_quitPrompt->setText(tr("Are you sure you want to quit capture?"));
    m_quitPrompt->setIcon(QMessageBox::Icon::Question);
    m_quitPrompt->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_quitPrompt->setDefaultButton(QMessageBox::No);

    auto* check = new QCheckBox(tr("Do not show this again"));
    m_quitPrompt->setCheckBox(check);

    // Call show() first, otherwise the correct geometry cannot be fetched
    // for centering the window on the screen
    m_quitPrompt->show();
    QRect position = m_quitPrompt->frameGeometry();
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(currentScreen->availableGeometry().center());
    m_quitPrompt->move(position.topLeft());
    m_quitPrompt->hide();

    QObject::connect(check, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setShowQuitPrompt(!checked);
    });
}

bool CaptureWidget::promptQuit()
{
    return m_quitPrompt->exec() == QMessageBox::Yes;
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
        const QRect twRect = toolWidgetCanvasRect();
        m_toolWidget->hide();
        delete m_toolWidget;
        m_toolWidget = nullptr;
        if (m_multiMonitor && !twRect.isEmpty()) {
            updateCanvas(twRect);
        }
    } else if (m_colorPicker && m_colorPicker->isVisible()) {
        const QRect cpRect = colorPickerCanvasRect();
        m_colorPicker->hide();
        if (m_multiMonitor && !cpRect.isEmpty()) {
            updateCanvas(cpRect);
        }
    } else {
        // close CaptureWidget
        if (m_config.showQuitPrompt()) {
            // need to show prompt
            if (m_quitPrompt->isHidden() && promptQuit()) {
                close();
            }
        } else {
            close();
        }
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
        const QRect twRect = toolWidgetCanvasRect();
        m_toolWidget->hide();
        delete m_toolWidget;
        m_toolWidget = nullptr;
        if (m_multiMonitor && !twRect.isEmpty()) {
            updateCanvas(twRect);
        }
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

void CaptureWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_multiMonitor && !m_closing) {
        for (ScreenOverlay* overlay : m_overlays) {
            overlay->syncWithScreen();
            overlay->showFullScreen();
        }
        if (ScreenOverlay* primary = primaryOverlay()) {
            primary->raise();
            primary->activateWindow();
        }
    }
}

void CaptureWidget::hideEvent(QHideEvent* event)
{
    if (m_multiMonitor) {
        for (ScreenOverlay* overlay : m_overlays) {
            overlay->hide();
        }
    }
    QWidget::hideEvent(event);
}

void CaptureWidget::closeEvent(QCloseEvent* event)
{
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    /* GNOME copy problem workaround, copy
       operation seems to work only when there
       is a visible window to retrieve the
       data from. On GNOME, the GUI should
       handle the copy operation, not the
       daemon.
    */
    const bool copyRequested =
      (m_context.request.tasks() & CaptureRequest::COPY);

    if (m_captureDone && copyRequested) {
        DesktopInfo desktopInfo;
        const bool needGnomeWorkaround =
          desktopInfo.waylandDetected() &&
          desktopInfo.windowManager() == DesktopInfo::GNOME;

        if (needGnomeWorkaround && !m_clipboardWorkaroundDone) {
            event->ignore();
            m_clipboardWorkaroundDone = true;
            m_context.request.removeTask(CaptureRequest::COPY);
            AbstractLogger::info()
              << "GNOME Wayland detected; keeping capture window alive until "
                 "clipboard data is fetched.";
            saveToClipboardGnomeWorkaround(pixmap(), this);
            return;
        }
    }
#endif

    m_closing = true;
    if (m_notifierBox) {
        disconnect(m_notifierBox, nullptr, this, nullptr);
    }
    for (ScreenOverlay* overlay : m_overlays) {
        overlay->hide();
    }
    for (ScreenOverlay* overlay : m_overlays) {
        overlay->close();
    }
    QWidget::closeEvent(event);
}

void CaptureWidget::paintEvent(QPaintEvent* paintEvent)
{
    Q_UNUSED(paintEvent)
    if (m_multiMonitor) {
        return;
    }
    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }
    paintCanvas(painter, rect());
}

void CaptureWidget::paintScreenOverlay(ScreenOverlay* overlay,
                                       QPaintEvent* paintEvent)
{
    QPainter painter(overlay);
    if (!painter.isActive()) {
        return;
    }
    painter.fillRect(paintEvent->rect(), Qt::black);
    painter.save();
    painter.translate(-overlay->canvasOffset());
    paintCanvas(painter, overlay->canvasRect());
    if (m_selection) {
        m_selection->renderSelection(painter);
    }
    if (m_toolWidget && m_toolWidget->isVisible() &&
        m_toolWidget->parentWidget() != overlay) {
        const QRect twRect = toolWidgetCanvasRect();
        if (overlay->canvasRect().intersects(twRect)) {
            painter.save();
            painter.translate(twRect.topLeft());
            m_toolWidget->render(
              &painter, QPoint(), QRegion(), QWidget::DrawChildren);
            painter.restore();
        }
    }
    if (m_colorPicker && m_colorPicker->isVisible() &&
        m_colorPicker->parentWidget() != overlay) {
        const QRect cpRect = colorPickerCanvasRect();
        if (overlay->canvasRect().intersects(cpRect)) {
            painter.save();
            painter.translate(cpRect.topLeft());
            m_colorPicker->renderPicker(painter);
            painter.restore();
        }
    }
    if (m_panelButtonDragging && m_panelToggleButton &&
        m_panelToggleButton->parentWidget() != overlay &&
        !m_panelButtonDragPixmap.isNull()) {
        const QRect btnRect(m_panelButtonCanvasPos,
                            m_panelToggleButton->size());
        if (overlay->canvasRect().intersects(btnRect)) {
            painter.drawPixmap(m_panelButtonCanvasPos, m_panelButtonDragPixmap);
        }
    }
    if (m_magnifier && !m_activeButton &&
        overlay->canvasRect().contains(m_context.mousePos)) {
        m_magnifier->renderAt(
          painter, m_context.mousePos, overlay->canvasRect());
    }
    painter.restore();
}

void CaptureWidget::paintCanvas(QPainter& painter, const QRect& viewBounds)
{
    GeneralConf::xywh_position position =
      static_cast<GeneralConf::xywh_position>(m_config.showSelectionGeometry());
    /* QPainter::save and restore is somewhat costly so we try to guess
       if we need to do it here. What that means is that if you add
       anything to the paintEvent and want to save/restore you should
       add a test to the below if statement -- also if you change
       any of the conditions that current trigger it you'll need to change here,
       too
    */
    const bool smoothScale =
      m_multiMonitor && !qFuzzyCompare(painter.device()->devicePixelRatioF(),
                                       m_context.screenshot.devicePixelRatio());
    bool save = false;
    if (smoothScale ||                             // mixed-DPI smoothing
        m_xywhDisplay ||                           // clause 1: xywh display
        m_displayGrid ||                           // clause 2: display grid
        (m_activeTool && m_mouseIsClicked) ||      // clause 3: tool/click
        (m_previewEnabled && activeButtonTool() && // clause 4: mouse preview
         m_activeButton->tool()->showMousePreview())) {
        painter.save();
        save = true;
    }
    if (smoothScale) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    }
    painter.drawPixmap(0, 0, m_context.screenshot);
    if (m_selection && m_xywhDisplay) {
        const QRect& selection = m_selection->geometry().normalized();
        const qreal scale = m_context.screenshot.devicePixelRatio();
        QRect xybox;
        QFontMetrics fm = painter.fontMetrics();

        QString xy =
          QString("%1x%2+%3+%4")
            .arg(QString::number(static_cast<int>(selection.width() * scale)),
                 QString::number(static_cast<int>(selection.height() * scale)),
                 QString::number(static_cast<int>(selection.left() * scale)),
                 QString::number(static_cast<int>(selection.top() * scale)));

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

        const auto scale{ m_context.screenshot.devicePixelRatio() };
        const QPoint selTopLeft = m_context.selection.topLeft() / scale;
        QPoint topLeft = m_multiMonitor
                           ? (selTopLeft + m_desktopGeometry.topLeft())
                           : mapToGlobal(selTopLeft);
        topLeft.rx() -= topLeft.x() % m_gridSize;
        topLeft.ry() -= topLeft.y() % m_gridSize;
        topLeft = m_multiMonitor ? (topLeft - m_desktopGeometry.topLeft())
                                 : mapFromGlobal(topLeft);

        const auto step{ m_gridSize / scale };
        const auto radius{ 1 * scale };

        for (int y = topLeft.y(); y < m_context.selection.bottom() / scale;
             y += step) {
            for (int x = topLeft.x(); x < m_context.selection.right() / scale;
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

    const bool hasFocus = m_multiMonitor
                            ? (QApplication::activeWindow() != nullptr)
                            : isActiveWindow();
    if (!hasFocus) {
        drawErrorMessage(
          tr("Flameshot has lost focus. Keyboard shortcuts won't "
             "work until you click somewhere."),
          &painter,
          viewBounds);
    } else if (m_configError) {
        drawErrorMessage(ConfigHandler().errorMessage(), &painter, viewBounds);
    } else if (m_configErrorResolved) {
        drawErrorMessage(tr("Configuration error resolved. Launch `flameshot "
                            "gui` again to apply it."),
                         &painter,
                         viewBounds);
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
    const QRect prevCpRect = colorPickerCanvasRect();
    QPoint pickerPos = pos;
    if (m_multiMonitor) {
        if (ScreenOverlay* host = overlayAtCanvasPos(pos)) {
            if (m_colorPicker->parentWidget() != host) {
                m_colorPicker->setParent(host);
            }
            pickerPos = pos - host->canvasOffset();
        }
    }
    m_colorPicker->move(pickerPos.x() - m_colorPicker->width() / 2,
                        pickerPos.y() - m_colorPicker->height() / 2);
    m_colorPicker->raise();
    m_colorPicker->show();
    if (m_multiMonitor) {
        updateCanvas(prevCpRect.united(colorPickerCanvasRect()));
    }
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
    if (!m_multiMonitor) {
        activateWindow();
    }
    m_startMove = false;
    m_startMovePos = QPoint();
    m_mousePressedPos = e->pos();
    m_activeToolOffsetToMouseOnStart = QPoint();
    if (m_colorPicker->isVisible()) {
        updateCursor();
        return;
    }
    if (m_multiMonitor && m_toolWidget &&
        toolWidgetCanvasRect().contains(e->pos())) {
        m_toolWidget->setFocus();
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
    if (m_toolWidget && !toolWidgetCanvasRect().contains(e->pos())) {
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
            if (!m_activeTool.isNull()) {
                m_panel->setToolWidget(m_activeTool->configurationWidget());
            }
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
        m_magnifier->setCursorPos(e->pos());
        if (!m_activeButton) {
            m_magnifier->show();
            if (m_multiMonitor) {
                updateCanvas();
            } else {
                m_magnifier->update();
            }
        } else {
            if (m_magnifier->isVisible()) {
                m_magnifier->hide();
                if (m_multiMonitor) {
                    updateCanvas();
                }
            }
        }
    }

    m_context.mousePos = e->pos();
    if (m_multiMonitor && m_colorPicker && m_colorPicker->isVisible()) {
        const QRect cpCanvas = colorPickerCanvasRect();
        m_colorPicker->updateHoverPosition(m_context.mousePos -
                                           cpCanvas.topLeft());
    }
    if (m_multiMonitor && m_toolWidget &&
        toolWidgetCanvasRect().contains(e->pos()) &&
        (e->buttons() & Qt::LeftButton)) {
        return;
    }
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
            updateCanvas(paddedUpdateRect(activeTool->boundingRect()));
            activeTool->move(e->pos() - m_activeToolOffsetToMouseOnStart);
            drawToolsData();
        }
    } else if (m_activeTool) {
        // drawing with a tool
        if (m_adjustmentButtonPressed) {
            m_activeTool->drawMoveWithAdjustment(e->pos());
        } else {
            QPoint drawPos = m_displayGrid ? snapToGrid(e->pos()) : e->pos();
            if (m_multiMonitor && m_toolWidget) {
                if (ScreenOverlay* host = overlayAtCanvasPos(drawPos)) {
                    if (m_toolWidget->parentWidget() != host) {
                        m_toolWidget->setParent(host);
                        m_toolWidget->show();
                        m_toolWidget->setFocus();
                    }
                    drawPos -= host->canvasOffset();
                }
            }
            m_activeTool->drawMove(drawPos);
        }
        // update drawing object
        updateTool(m_activeTool);
        // Hides the buttons under the mouse. If the mouse leaves, it shows
        // them.
        if (m_buttonHandler->buttonsAreInside()) {
            QPoint buttonMousePos = m_context.mousePos;
            if (m_multiMonitor && !m_toolbarButtons.isEmpty()) {
                if (ScreenOverlay* host = overlayForParent(
                      m_toolbarButtons.first()->parentWidget())) {
                    buttonMousePos -= host->canvasOffset();
                }
            }
            const bool containsMouse =
              m_buttonHandler->contains(buttonMousePos);
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
        const QRect cpRect = colorPickerCanvasRect();
        // Color picker
        if (m_colorPicker->isVisible() && m_panel->activeLayerIndex() >= 0 &&
            m_context.color.isValid()) {
            pushObjectsStateToUndoStack();
        }
        m_colorPicker->setNewColor();
        m_colorPicker->hide();
        if (m_multiMonitor && !cpRect.isEmpty()) {
            updateCanvas(cpRect);
        }
        if (!m_context.color.isValid()) {
            m_context.color = ConfigHandler().drawColor();
            syncPanelToOverlay(overlayAtCanvasPos(m_context.mousePos));
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

    int offset = m_notifierBox->width() / 4;
    if (m_multiMonitor) {
        if (ScreenOverlay* host = overlayAtCanvasPos(m_context.mousePos)) {
            if (m_notifierBox->parentWidget() != host) {
                m_notifierBox->setParent(host);
            }
            m_notifierBox->move(offset, offset);
        }
    } else {
        QScreen* topLeftScreen = QGuiAppCurrentScreen().currentScreen();
        QPoint topLeft(0, 0);
        if (topLeftScreen) {
            topLeft = topLeftScreen->geometry().topLeft();
        } else {
            QScreen* primary = QGuiApplication::primaryScreen();
            if (primary) {
                topLeft = primary->geometry().topLeft();
            }
        }
        m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    }
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
    if (qAbs(e->angleDelta().y()) >= MOUSE_WHEEL_TRESHOLD) {
        auto const delta =
          qMax(qMin(e->angleDelta().y() / MOUSE_WHEEL_TRESHOLD, 1), -1);
        if (activeButtonTool() &&
            activeButtonTool()->handleMouseWheelEvent(
              delta, m_adjustmentButtonPressed, m_context)) {
            repaintCanvas();
            return;
        }
        toolSizeOffset = delta;
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
    if (m_multiMonitor) {
        m_context.widgetOffset = m_desktopGeometry.topLeft();
        return;
    }
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    if (!m_context.fullscreen) {
        m_panel->setFixedHeight(height());
        m_buttonHandler->updateScreenRegions(rect());
    }
}

void CaptureWidget::moveEvent(QMoveEvent* e)
{
    QWidget::moveEvent(e);
    if (m_multiMonitor) {
        m_context.widgetOffset = m_desktopGeometry.topLeft();
        return;
    }
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    // If a compositor moved the window to the wrong screen, move it back
    if (m_context.fullscreen && m_selectedScreen &&
        geometry().topLeft() != m_selectedScreen->geometry().topLeft()) {
        move(m_selectedScreen->geometry().topLeft());
    }
#endif
}

void CaptureWidget::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::ActivationChange) {
        QPoint bottomRight = rect().bottomRight();
        // Update the message in the bottom right corner. A rough estimate is
        // used for the update rect
        updateCanvas(QRect(bottomRight - QPoint(1000, 200), bottomRight));
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
    // Use widget-local coordinates (rect()) for all child widgets
    // Child widgets use parent-relative coordinate system, not global screen
    // coords
    QWidget* panelHost = this;
    if (m_multiMonitor) {
        if (ScreenOverlay* primary = primaryOverlay()) {
            panelHost = primary;
        }
    }
    QRect panelRect = panelHost->rect();

    if (ConfigHandler().showSidePanelButton()) {
        m_panelToggleButton =
          new OrientablePushButton(tr("Tool Settings"), panelHost);
        m_panelToggleButton->installEventFilter(m_eventFilter);
        m_panelToggleButton->setColor(m_uiColor);
        m_panelToggleButton->setOrientation(
          OrientablePushButton::VerticalBottomToTop);
        m_panelToggleButton->adjustSize();
#if defined(Q_OS_MACOS)
        m_panelToggleButton->move(
          0, (panelRect.height() - m_panelToggleButton->height()) / 2);
#else
        // panelRect is already adjusted for DPR, so centering calculations work
        // correctly
        m_panelToggleButton->move(
          panelRect.x(),
          panelRect.y() +
            (panelRect.height() - m_panelToggleButton->height()) / 2);
#endif
        m_panelToggleButton->setCursor(Qt::ArrowCursor);
        if (m_multiMonitor) {
            m_panelToggleButton->installEventFilter(this);
        } else {
            (new DraggableWidgetMaker(this))
              ->makeDraggable(m_panelToggleButton);
        }
        connect(m_panelToggleButton,
                &QPushButton::clicked,
                this,
                &CaptureWidget::togglePanel);
    }

    m_panel = new UtilityPanel(this);
    if (panelHost != this) {
        m_panel->setParent(panelHost);
    }
    m_panel->hide();
    m_panel->installEventFilter(m_eventFilter);
#if defined(Q_OS_MACOS)
    m_panel->setFixedWidth(static_cast<int>(m_colorPicker->width() * 1.5));
    m_panel->setFixedHeight(height());
#else
    // Panel uses widget-local coordinates (parent-relative)
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
    connect(
      m_sidePanel, &SidePanelWidget::showPanel, m_panel, &UtilityPanel::show);
    connect(
      m_sidePanel, &SidePanelWidget::hidePanel, m_panel, &UtilityPanel::hide);
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
    if (m_closing || !ConfigHandler().checkForUpdates()) {
        // option check for updates disabled
        return;
    }
    QWidget* host = this;
    if (m_multiMonitor) {
        if (ScreenOverlay* primary = primaryOverlay()) {
            host = primary;
        }
    }
    if (nullptr == m_updateNotificationWidget) {
        m_updateNotificationWidget =
          new UpdateNotificationWidget(host, appLatestVersion, appLatestUrl);
    }
    if (m_multiMonitor && host != this) {
        int ax = (host->width() - m_updateNotificationWidget->width()) / 2;
        m_updateNotificationWidget->setParent(host);
        m_updateNotificationWidget->installEventFilter(m_eventFilter);
        m_updateNotificationWidget->move(ax, 0);
        m_updateNotificationWidget->show();
        return;
    }
#if defined(Q_OS_MACOS)
    int ax = (width() - m_updateNotificationWidget->width()) / 2;
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
        if (m_multiMonitor) {
            updateCanvas();
        }
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
            if (m_multiMonitor) {
                syncButtonsToOverlays();
            }
            m_buttonHandler->show();
        } else {
            m_buttonHandler->hide();
        }
    });
    connect(m_selection, &SelectionWidget::visibilityChanged, this, [this]() {
        if (!m_selection->isVisible() && !m_helpMessage.isEmpty()) {
            OverlayMessage::push(m_helpMessage);
        }
        if (m_multiMonitor) {
            updateCanvas();
        }
    });
    if (!initialSelection.isNull()) {
        const qreal scale = m_context.screenshot.devicePixelRatio();
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
                const QRect oldTwRect = toolWidgetCanvasRect();
                m_toolWidget->hide();
                delete m_toolWidget;
                m_toolWidget = nullptr;
                if (m_multiMonitor && !oldTwRect.isEmpty()) {
                    updateCanvas(oldTwRect);
                }
            }
            m_toolWidget = m_activeTool->widget();
            if (m_toolWidget) {
                if (m_multiMonitor) {
                    if (ScreenOverlay* host =
                          overlayAtCanvasPos(m_context.mousePos)) {
                        m_toolWidget->setParent(host);
                        m_toolWidget->installEventFilter(m_eventFilter);
                        m_toolWidget->move(m_context.mousePos -
                                           host->canvasOffset());
                        host->activateWindow();
                    } else {
                        makeChild(m_toolWidget);
                        m_toolWidget->move(m_context.mousePos);
                    }
                    m_prevToolWidgetCanvasRect = toolWidgetCanvasRect();
                    m_toolWidget->installEventFilter(this);
                    if (auto* edit = qobject_cast<QTextEdit*>(m_toolWidget)) {
                        connect(edit, &QTextEdit::textChanged, this, [this]() {
                            if (m_multiMonitor) {
                                const QRect currentRect =
                                  toolWidgetCanvasRect();
                                updateCanvas(m_prevToolWidgetCanvasRect.united(
                                  currentRect));
                                m_prevToolWidgetCanvasRect = currentRect;
                            }
                        });
                        connect(edit,
                                &QTextEdit::cursorPositionChanged,
                                this,
                                [this]() {
                                    if (m_multiMonitor) {
                                        updateCanvas(toolWidgetCanvasRect());
                                    }
                                });
                    }
                } else {
                    makeChild(m_toolWidget);
                    m_toolWidget->move(m_context.mousePos);
                }
                m_toolWidget->show();
                m_toolWidget->setFocus();
                if (m_multiMonitor) {
                    updateCanvas(toolWidgetCanvasRect());
                }
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
        case CaptureTool::REQ_COMMIT_CURRENT_TOOL: {
            const bool editingExistingTool =
              m_activeTool && m_activeTool->editMode();
            if (commitCurrentTool()) {
                if (editingExistingTool) {
                    m_panel->setToolWidget(nullptr);
                }
                drawToolsData();
                updateLayersPanel();
            }
            break;
        }
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

    // Persist the new size straight away. This was previously deferred until
    // the NotifierBox finished hiding, which meant a size set from the side
    // panel was never saved at all, since that path never shows the notifier,
    // and a size set with the wheel or the keyboard was lost whenever the
    // capture was completed before the notifier's timeout had elapsed.
    m_config.setToolSize(activeButtonToolType(), t);

    CaptureTool* tool = activeButtonTool();
    if (tool && tool->showMousePreview()) {
        setCursor(Qt::BlankCursor);
        if (m_multiMonitor) {
            for (ScreenOverlay* overlay : m_overlays) {
                overlay->setCursor(Qt::BlankCursor);
            }
        }
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
    repaintCanvas();
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
        updateCanvas(
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
    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_GRAB_COLOR")),
                this,
                SLOT(startColorGrab()));

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

    newShortcut(QKeySequence(ConfigHandler().shortcut("TYPE_CANCEL")),
                this,
                SLOT(cancel()));

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
        m_sizeIndButton->setText(
          QStringLiteral("%1\n%2").arg(selection.width(), selection.height()));
    }
}

void CaptureWidget::updateCursor()
{
    const QPoint mousePos =
      m_multiMonitor ? m_context.mousePos : mapFromGlobal(QCursor::pos());
    if (m_colorPicker && m_colorPicker->isVisible()) {
        setCursor(Qt::ArrowCursor);
    } else if (m_activeButton != nullptr &&
               activeButtonToolType() != CaptureTool::TYPE_MOVESELECTION) {
        setCursor(Qt::CrossCursor);
    } else if (m_selection->getMouseSide(mousePos) !=
               SelectionWidget::NO_SIDE) {
        setCursor(m_selection->cursor());
    } else if (activeButtonToolType() == CaptureTool::TYPE_MOVESELECTION) {
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::CrossCursor);
    }
    if (m_multiMonitor) {
        for (ScreenOverlay* overlay : m_overlays) {
            overlay->setCursor(cursor());
        }
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
    updateCanvas(previewRect);
    updateCanvas(toolObjectRect);
    updateCanvas(oldPreviewRect);
    updateCanvas(oldToolObjectRect);

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
    for (const auto& toolItem : m_captureToolObjects.captureToolObjects()) {
        processPixmapWithTool(&pixmapItem, toolItem);
        updateCanvas(paddedUpdateRect(toolItem->boundingRect()));
    }

    m_context.screenshot = pixmapItem;
    if (drawSelection) {
        drawObjectSelection();
    }
    if (m_multiMonitor) {
        updateCanvas();
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
        updateCanvas(paddedUpdateRect(toolItem->boundingRect()));
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
    QPoint snapPoint = m_multiMonitor ? (point + m_desktopGeometry.topLeft())
                                      : mapToGlobal(point);

    snapPoint.setX((qRound(snapPoint.x() / double(m_gridSize)) * m_gridSize));
    snapPoint.setY((qRound(snapPoint.y() / double(m_gridSize)) * m_gridSize));

    return m_multiMonitor ? (snapPoint - m_desktopGeometry.topLeft())
                          : mapFromGlobal(snapPoint);
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
    QList<QKeySequence> seqs;
    QString strKey = key.toString();
    if (strKey.contains("Enter") || strKey.contains("Return")) {
        strKey.replace("Enter", "Return");
        seqs << QKeySequence(strKey);
        strKey.replace("Return", "Enter");
        seqs << QKeySequence(strKey);
    } else {
        seqs << key;
    }

    if (m_multiMonitor && !m_overlays.isEmpty()) {
        for (const QKeySequence& seq : seqs) {
            for (ScreenOverlay* overlay : m_overlays) {
                auto* sc = new QShortcut(seq, overlay);
                if (slot) {
                    connect(sc, SIGNAL(activated()), parent, slot);
                }
                shortcuts << sc;
            }
        }
    } else {
        for (const QKeySequence& seq : seqs) {
            shortcuts << new QShortcut(seq, parent, slot);
        }
    }
    return shortcuts;
}

void CaptureWidget::togglePanel()
{
    if (m_multiMonitor && !m_panel->isVisible()) {
        ScreenOverlay* host = nullptr;
        if (sender() == m_panelToggleButton && m_panelToggleButton) {
            host = overlayForParent(m_panelToggleButton->parentWidget());
        }
        if (!host) {
            host = overlayAtCanvasPos(m_context.mousePos);
        }
        syncPanelToOverlay(host);
    }
    m_panel->toggle();
}

void CaptureWidget::childEnter()
{
    if (m_closing) {
        return;
    }
    m_previewEnabled = false;
    updateTool(activeButtonTool());
}

void CaptureWidget::childLeave()
{
    if (m_closing) {
        return;
    }
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
    updateCanvas();
    updateLayersPanel();

    restoreCircleCountState();
}

void CaptureWidget::cancel()
{
    if (m_activeButton != nullptr) {
        uncheckActiveTool();
    }
    if (m_panel) {
        m_panel->setActiveLayer(-1);
    }
    if (m_toolWidget) {
        m_toolWidget->hide();
        delete m_toolWidget;
        m_toolWidget = nullptr;
    }
    m_selection->hide();
    emit m_selection->geometrySettled();
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
    return { qRound(r.left() * devicePixelRatio),
             qRound(r.top() * devicePixelRatio),
             qRound(r.width() * devicePixelRatio),
             qRound(r.height() * devicePixelRatio) };
}

QRect CaptureWidget::paddedUpdateRect(const QRect& r) const
{
    if (r.isNull()) {
        return r;
    } else {
        return r + QMargins(20, 20, 20, 20);
    }
}

void CaptureWidget::drawErrorMessage(const QString& msg,
                                     QPainter* painter,
                                     const QRect& bounds)
{
    auto textRect = painter->fontMetrics().boundingRect(msg);
    int w = textRect.width(), h = textRect.height();
    textRect = {
        bounds.x() + bounds.width() - w - 10,
        bounds.y() + bounds.height() - h - 5,
        w + 100,
        h + 100,
    };
    const QPoint cursorPos =
      m_multiMonitor ? m_context.mousePos
                     : QCursor::pos(QGuiAppCurrentScreen().currentScreen());

    if (!textRect.contains(cursorPos)) {
        QColor textColor(Qt::white);
        painter->setPen(textColor);
        painter->drawText(textRect, msg);
    }
}

void CaptureWidget::drawInactiveRegion(QPainter* painter)
{
    painter->save();
    QColor overlayColor(0, 0, 0, m_opacity);
    painter->setBrush(overlayColor);
    QRect r;
    if (m_selection->isVisible()) {
        r = m_selection->geometry().normalized();
    }
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter->setClipRegion(grey, Qt::IntersectClip);
    painter->drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter->restore();
}

void CaptureWidget::forwardOverlayMouseEvent(ScreenOverlay* overlay,
                                             QMouseEvent* event)
{
    if (m_forwardingOverlayEvent || m_closing) {
        return;
    }
    m_forwardingOverlayEvent = true;

    const QPointF rawCanvas =
      event->position() + QPointF(overlay->canvasOffset());
    const QPointF canvasPos(
      qBound(0.0, rawCanvas.x(), qMax(0.0, static_cast<qreal>(width() - 1))),
      qBound(0.0, rawCanvas.y(), qMax(0.0, static_cast<qreal>(height() - 1))));
    const QPointF globalPos = canvasPos + QPointF(m_desktopGeometry.topLeft());

    QMouseEvent mapped(event->type(),
                       canvasPos,
                       canvasPos,
                       globalPos,
                       event->button(),
                       event->buttons(),
                       event->modifiers(),
                       event->pointingDevice());
    QCoreApplication::sendEvent(this, &mapped);
    m_forwardingOverlayEvent = false;
}

void CaptureWidget::updateCanvas(const QRect& r)
{
    if (m_closing) {
        return;
    }
    if (!m_multiMonitor) {
        if (r.isNull()) {
            update();
        } else {
            update(r);
        }
        return;
    }
    for (ScreenOverlay* overlay : m_overlays) {
        if (r.isNull()) {
            overlay->update();
        } else {
            const QRect local = r.translated(-overlay->canvasOffset())
                                  .intersected(overlay->rect());
            if (!local.isEmpty()) {
                overlay->update(local);
            }
        }
    }
}

void CaptureWidget::repaintCanvas()
{
    if (m_closing) {
        return;
    }
    if (!m_multiMonitor) {
        repaint();
        return;
    }
    for (ScreenOverlay* overlay : m_overlays) {
        overlay->repaint();
    }
}

ScreenOverlay* CaptureWidget::primaryOverlay() const
{
    if (m_overlays.isEmpty()) {
        return nullptr;
    }
    QScreen* primary = QGuiApplication::primaryScreen();
    for (ScreenOverlay* overlay : m_overlays) {
        if (overlay->targetScreen() == primary) {
            return overlay;
        }
    }
    return m_overlays.first();
}

ScreenOverlay* CaptureWidget::overlayAtCanvasPos(const QPoint& pos) const
{
    if (m_overlays.isEmpty()) {
        return nullptr;
    }
    for (ScreenOverlay* overlay : m_overlays) {
        if (overlay->canvasRect().contains(pos)) {
            return overlay;
        }
    }
    ScreenOverlay* best = m_overlays.first();
    qint64 bestDistSq = std::numeric_limits<qint64>::max();
    for (ScreenOverlay* overlay : m_overlays) {
        const QRect r = overlay->canvasRect();
        qint64 dx = 0;
        if (pos.x() < r.left()) {
            dx = r.left() - pos.x();
        } else if (pos.x() > r.right()) {
            dx = pos.x() - r.right();
        }
        qint64 dy = 0;
        if (pos.y() < r.top()) {
            dy = r.top() - pos.y();
        } else if (pos.y() > r.bottom()) {
            dy = pos.y() - r.bottom();
        }
        const qint64 distSq = dx * dx + dy * dy;
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            best = overlay;
        }
    }
    return best;
}

ScreenOverlay* CaptureWidget::overlayForRect(const QRect& r) const
{
    if (m_overlays.isEmpty()) {
        return nullptr;
    }
    ScreenOverlay* best = nullptr;
    qint64 bestArea = 0;
    const QRect norm = r.normalized();
    for (ScreenOverlay* overlay : m_overlays) {
        const QRect inter = overlay->canvasRect().intersected(norm);
        if (!inter.isEmpty()) {
            const qint64 area =
              static_cast<qint64>(inter.width()) * inter.height();
            if (area > bestArea) {
                bestArea = area;
                best = overlay;
            }
        }
    }
    return best ? best : overlayAtCanvasPos(norm.center());
}

ScreenOverlay* CaptureWidget::overlayForParent(QWidget* parent) const
{
    for (ScreenOverlay* overlay : m_overlays) {
        if (overlay == parent) {
            return overlay;
        }
    }
    return nullptr;
}

void CaptureWidget::syncButtonsToOverlays()
{
    if (!m_multiMonitor) {
        return;
    }
    for (CaptureToolButton* b : m_toolbarButtons) {
        const QPoint canvasPos = b->pos();
        const QPoint center =
          canvasPos + QPoint(b->width() / 2, b->height() / 2);
        ScreenOverlay* host = overlayAtCanvasPos(center);
        if (!host) {
            continue;
        }
        if (b->parentWidget() != host) {
            const bool wasVisible = b->isVisible();
            b->setParent(host);
            if (wasVisible) {
                b->show();
            }
        }
        b->move(canvasPos - host->canvasOffset());
    }
}

void CaptureWidget::syncPanelToOverlay(ScreenOverlay* host)
{
    if (!m_multiMonitor || !m_panel || !host) {
        return;
    }
    if (m_panel->parentWidget() != host) {
        m_panel->setParent(host);
    }
    m_panel->setGeometry(
      0, 0, static_cast<int>(m_colorPicker->width() * 1.5), host->height());
    m_panel->raise();
    host->activateWindow();
}

void CaptureWidget::onOverlayResized(ScreenOverlay* overlay)
{
    if (!m_multiMonitor || !overlay) {
        return;
    }
    if (m_buttonHandler && !m_overlays.isEmpty()) {
        QVector<QRect> areas;
        areas.reserve(m_overlays.size());
        for (ScreenOverlay* ov : m_overlays) {
            areas.append(ov->canvasRect());
        }
        m_buttonHandler->updateScreenRegions(areas);
    }
    if (m_panelToggleButton && !m_panelButtonMoved &&
        m_panelToggleButton->parentWidget() == overlay) {
        m_panelToggleButton->adjustSize();
        m_panelToggleButton->move(
          0, (overlay->height() - m_panelToggleButton->height()) / 2);
    }
    if (m_panel && m_colorPicker && m_panel->parentWidget() == overlay) {
        m_panel->setGeometry(0,
                             0,
                             static_cast<int>(m_colorPicker->width() * 1.5),
                             overlay->height());
    }
}

bool CaptureWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (m_multiMonitor && m_toolWidget && obj == m_toolWidget) {
        switch (event->type()) {
            case QEvent::Resize:
            case QEvent::Move: {
                const QRect currentRect = toolWidgetCanvasRect();
                updateCanvas(m_prevToolWidgetCanvasRect.united(currentRect));
                m_prevToolWidgetCanvasRect = currentRect;
                break;
            }
            default:
                break;
        }
    }
    if (m_multiMonitor && m_panelToggleButton && obj == m_panelToggleButton) {
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                auto* me = static_cast<QMouseEvent*>(event);
                m_panelButtonPressing = false;
                m_panelButtonDragging = false;
                if (me->button() == Qt::LeftButton) {
                    if (ScreenOverlay* host = overlayForParent(
                          m_panelToggleButton->parentWidget())) {
                        host->activateWindow();
                        m_panelButtonPressing = true;
                        m_panelButtonGrabOffset = me->position().toPoint();
                        m_panelButtonCanvasPos =
                          host->canvasOffset() + m_panelToggleButton->pos();
                        m_panelButtonPressCanvasPos =
                          m_panelButtonCanvasPos + m_panelButtonGrabOffset;
                    }
                }
                break;
            }
            case QEvent::MouseMove: {
                if (m_panelButtonPressing) {
                    auto* me = static_cast<QMouseEvent*>(event);
                    if (ScreenOverlay* host = overlayForParent(
                          m_panelToggleButton->parentWidget())) {
                        const QPoint cursorCanvasPos =
                          host->canvasOffset() + m_panelToggleButton->pos() +
                          me->position().toPoint();
                        m_context.mousePos = cursorCanvasPos;
                        if (!m_panelButtonDragging &&
                            (cursorCanvasPos - m_panelButtonPressCanvasPos)
                                .manhattanLength() > 3) {
                            m_panelButtonDragging = true;
                            m_panelButtonMoved = true;
                            m_panelButtonDragPixmap =
                              m_panelToggleButton->grab();
                        }
                        if (m_panelButtonDragging) {
                            const QSize btnSize = m_panelToggleButton->size();
                            const QRect oldRect(m_panelButtonCanvasPos,
                                                btnSize);
                            QPoint newPos =
                              cursorCanvasPos - m_panelButtonGrabOffset;
                            newPos.setX(
                              qBound(0,
                                     newPos.x(),
                                     qMax(0, width() - btnSize.width())));
                            newPos.setY(
                              qBound(0,
                                     newPos.y(),
                                     qMax(0, height() - btnSize.height())));
                            m_panelButtonCanvasPos = newPos;
                            m_panelToggleButton->move(newPos -
                                                      host->canvasOffset());
                            const QRect newRect(m_panelButtonCanvasPos,
                                                btnSize);
                            updateCanvas(
                              oldRect.united(newRect).adjusted(-8, -8, 8, 8));
                        }
                    }
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                if (m_panelButtonPressing) {
                    m_panelButtonPressing = false;
                    if (m_panelButtonDragging) {
                        m_panelButtonDragging = false;
                        m_panelButtonDragPixmap = QPixmap();
                        const QSize btnSize = m_panelToggleButton->size();
                        const QPoint center =
                          m_panelButtonCanvasPos +
                          QPoint(btnSize.width() / 2, btnSize.height() / 2);
                        if (ScreenOverlay* targetHost =
                              overlayAtCanvasPos(center)) {
                            const QRect hr = targetHost->canvasRect();
                            m_panelButtonCanvasPos.setX(
                              qBound(hr.left(),
                                     m_panelButtonCanvasPos.x(),
                                     qMax(hr.left(),
                                          hr.right() + 1 - btnSize.width())));
                            m_panelButtonCanvasPos.setY(
                              qBound(hr.top(),
                                     m_panelButtonCanvasPos.y(),
                                     qMax(hr.top(),
                                          hr.bottom() + 1 - btnSize.height())));
                            if (m_panelToggleButton->parentWidget() !=
                                targetHost) {
                                m_panelToggleButton->setParent(targetHost);
                            }
                            m_panelToggleButton->move(
                              m_panelButtonCanvasPos -
                              targetHost->canvasOffset());
                            m_panelToggleButton->setDown(false);
                            m_panelToggleButton->show();
                            m_panelToggleButton->raise();
                            if (m_panel && m_panel->isVisible() &&
                                m_panel->parentWidget() == targetHost) {
                                m_panel->raise();
                            }
                            targetHost->activateWindow();
                        }
                        updateCanvas();
                        event->ignore();
                        return true;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

QRect CaptureWidget::toolWidgetCanvasRect() const
{
    if (!m_toolWidget) {
        return {};
    }
    QRect geom = m_toolWidget->geometry();
    if (m_multiMonitor) {
        for (ScreenOverlay* overlay : m_overlays) {
            if (m_toolWidget->parentWidget() == overlay) {
                geom.translate(overlay->canvasOffset());
                break;
            }
        }
    }
    return geom;
}

QRect CaptureWidget::colorPickerCanvasRect() const
{
    if (!m_colorPicker) {
        return {};
    }
    QRect geom = m_colorPicker->geometry();
    if (m_multiMonitor) {
        for (ScreenOverlay* overlay : m_overlays) {
            if (m_colorPicker->parentWidget() == overlay) {
                geom.translate(overlay->canvasOffset());
                break;
            }
        }
    }
    return geom;
}
