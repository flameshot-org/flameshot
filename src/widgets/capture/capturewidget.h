// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#pragma once

#include "tools/capturecontext.h"
#include "tools/capturetool.h"
#include "utils/confighandler.h"
#include "utils/resizehandles.h"
#include "widgets/capture/buttonhandler.h"
#include "widgets/capture/capturetoolbutton.h"
#include "widgets/capture/capturetoolobjects.h"
#include "widgets/capture/magnifierwidget.h"
#include "widgets/capture/selectionwidget.h"

#include <QMessageBox>
#include <QPointer>
#include <QTimer>
#include <QUndoStack>
#include <QWidget>

class QLabel;
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QScreen;
class QShortcut;
class QNetworkAccessManager;
class QNetworkReply;
class ColorPicker;
class NotifierBox;
class HoverEventFilter;
#if !defined(DISABLE_UPDATE_CHECKER)
class UpdateNotificationWidget;
#endif
class UtilityPanel;
class SidePanelWidget;
class OverlayMessage;

class CaptureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureWidget(const CaptureRequest& req,
                           bool fullScreen = true,
                           QWidget* parent = nullptr);
    ~CaptureWidget();

    QPixmap pixmap();
    void setCaptureToolObjects(const CaptureToolObjects& captureToolObjects);
#if !defined(DISABLE_UPDATE_CHECKER)
    void showAppUpdateNotification(const QString& appLatestVersion,
                                   const QString& appLatestUrl);
#endif

    /// Monitor this widget was built for, or -1 when none was pre-selected.
    int monitorIndex() const;

    /// Suppress the captureFailed() this widget emits on destruction. Used for
    /// the widgets discarded once the user commits to another display; without
    /// it each one would abort the whole application.
    void discardSilently();

    /// Only the armed widget shows the help overlay and magnifier; the others
    /// dim harder, so the display the pointer is on is obvious.
    void setArmed(bool armed);
    bool isArmed() const { return m_armed; }

    /// Make this widget's shortcuts fire from any window of the application
    /// when @p active, and not at all otherwise. Wayland keeps keyboard focus
    /// on whichever display had it, so keys must reach the armed one some
    /// other way. Exactly one widget may be active: two application-wide
    /// copies of a key are ambiguous and neither fires.
    void setSharedShortcutsActive(bool active);
    /// Back to shortcuts that fire only while this window has focus.
    void restoreWindowShortcuts();

public slots:
    bool commitCurrentTool();
    void deleteToolWidgetOrClose();

signals:
    void colorChanged(const QColor& c);
    void toolSizeChanged(int size);
    /// The pointer moved onto this widget's display.
    void pointerEnteredMonitor(int monitorIndex);
    /// First press: the user committed to this display.
    void editingStarted(int monitorIndex);

private slots:
    void undo();
    void redo();
    void cancel();
    void togglePanel();
    void childEnter();
    void childLeave();

    void deleteCurrentTool();

    void setState(CaptureToolButton* b);
    void handleToolSignal(CaptureTool::Request r);
    void handleButtonLeftClick(CaptureToolButton* b);
    void handleButtonRightClick(CaptureToolButton* b);
    void setDrawColor(const QColor& c);
    void onToolSizeChanged(int size);
    void onToolSizeSettled(int size);
    void updateActiveLayer(int layer);
    void onMoveCaptureToolUp(int captureToolIndex);
    void onMoveCaptureToolDown(int captureToolIndex);
    void selectAll();
    void xywhTick();
    void onDisplayGridChanged(bool display);
    void onGridSizeChanged(int size);

    void startColorGrab();

public:
    void removeToolObject(int index = -1);
    void showxywh();

protected:
    void paintEvent(QPaintEvent* paintEvent) override;
    void enterEvent(QEnterEvent* enterEvent) override;
    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* keyEvent) override;
    void keyReleaseEvent(QKeyEvent* keyEvent) override;
    void wheelEvent(QWheelEvent* wheelEvent) override;
    void resizeEvent(QResizeEvent* resizeEvent) override;
    void moveEvent(QMoveEvent* moveEvent) override;
    void changeEvent(QEvent* changeEvent) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void pushObjectsStateToUndoStack();
    void releaseActiveTool();
    void uncheckActiveTool();
    int selectToolItemAtPos(const QPoint& pos);
    void showColorPicker(const QPoint& pos);
    bool startDrawObjectTool(const QPoint& pos);
    QPointer<CaptureTool> activeToolObject();
    void initContext(bool fullscreen, const CaptureRequest& req);
    void initPanel();
    void initSelection();
    void initShortcuts();
    void initButtons();
    void initHelpMessage();
    void initQuitPrompt();
    void updateSizeIndicator();
    void updateCursor();
    ResizeHandles::Handle resizeHandleAt(const QPoint& pos);
    void updateSelectionState();
    void updateTool(CaptureTool* tool);
    void updateLayersPanel();
    bool promptQuit();
    void pushToolToStack();
    void makeChild(QWidget* w);
    void restoreCircleCountState();

    QList<QShortcut*> newShortcut(const QKeySequence& key,
                                  QWidget* parent,
                                  const char* slot);

    void setToolSize(int size);

    QRect extendedSelection() const;
    QRect extendedRect(const QRect& r) const;
    QRect paddedUpdateRect(const QRect& r) const;
    void drawErrorMessage(const QString& msg, QPainter* painter);
    void drawInactiveRegion(QPainter* painter);
    void drawToolsData(bool drawSelection = true);
    void drawObjectSelection();

    void processPixmapWithTool(QPixmap* pixmap, CaptureTool* tool);

    CaptureTool* activeButtonTool() const;
    CaptureTool::Type activeButtonToolType() const;

    QPoint snapToGrid(const QPoint& point) const;

    ////////////////////////////////////////
    // Class members

    // Context information
    CaptureContext m_context;
    QScreen* m_selectedScreen = nullptr;

    // Main ui color
    QColor m_uiColor;
    // Secondary ui color
    QColor m_contrastUiColor;

    // Outside selection opacity
    int m_opacity;
    int m_toolSizeByKeyboard;

    // utility flags
    bool m_mouseIsClicked;
    bool m_newSelection;
    bool m_movingSelection;
    bool m_captureDone;
    bool m_previewEnabled;
    bool m_adjustmentButtonPressed;
    bool m_configError;
    bool m_configErrorResolved;
    bool m_discardSilently = false;
    bool m_armed = true;
    // The selection was hidden by un-arming and is shown again on arming.
    bool m_selectionHiddenWhileUnarmed = false;
    int m_armedOpacity = 0;
    // This widget's own overlay, and the "Tool Settings" toggle, both hidden
    // while the display is unarmed.
    OverlayMessage* m_overlay = nullptr;
    QWidget* m_panelToggleButton = nullptr;

#if !defined(DISABLE_UPDATE_CHECKER)
    UpdateNotificationWidget* m_updateNotificationWidget;
#endif
    quint64 m_lastMouseWheel;
    QPointer<CaptureToolButton> m_sizeIndButton;
    // Last pressed button
    QPointer<CaptureToolButton> m_activeButton;
    QPointer<CaptureTool> m_activeTool;
    bool m_activeToolIsMoved;
    QPointer<QWidget> m_toolWidget;
    QPointer<QMessageBox> m_quitPrompt;

    ButtonHandler* m_buttonHandler;
    UtilityPanel* m_panel;
    SidePanelWidget* m_sidePanel;
    ColorPicker* m_colorPicker;
    ConfigHandler m_config;
    NotifierBox* m_notifierBox;
    HoverEventFilter* m_eventFilter;
    SelectionWidget* m_selection;
    MagnifierWidget* m_magnifier;
    QString m_helpMessage;

    SelectionWidget::SideType m_mouseOverHandle;

    QMap<CaptureTool::Type, CaptureTool*> m_tools;
    CaptureToolObjects m_captureToolObjects;
    CaptureToolObjects m_captureToolObjectsBackup;

    QPoint m_mousePressedPos;
    QPoint m_activeToolOffsetToMouseOnStart;

    // XYWH display position and timer
    bool m_xywhDisplay;
    QTimer m_xywhTimer;

    QUndoStack m_undoStack;

    bool m_existingObjectIsChanged;

    // For start moving after more than X offset
    QPoint m_startMovePos;
    bool m_startMove;

    // Resize handle being dragged, and the object it belongs to. Delete or
    // undo mid-drag replaces the object; the drag then stays cancelled until
    // the button is released.
    ResizeHandles::Handle m_resizeHandle{ ResizeHandles::None };
    QPointer<CaptureTool> m_resizeTool;

    // Grid
    bool m_displayGrid{ false };
    int m_gridSize{ 10 };

    bool m_clipboardWorkaroundDone{ false };
};
