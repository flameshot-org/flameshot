// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#pragma once

#include "buttonhandler.h"
#include "capturetoolbutton.h"
#include "capturetoolobjects.h"
#include "src/tools/capturecontext.h"
#include "src/tools/capturetool.h"
#include "src/utils/confighandler.h"
#include "src/widgets/capture/selectionwidget.h"
#include <QPointer>
#include <QUndoStack>
#include <QWidget>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QNetworkAccessManager;
class QNetworkReply;
class ColorPicker;
class NotifierBox;
class HoverEventFilter;
class UpdateNotificationWidget;
class UtilityPanel;
class SidePanelWidget;

class CaptureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureWidget(uint id = 0,
                           const QString& savePath = QString(),
                           bool fullScreen = true,
                           QWidget* parent = nullptr);
    ~CaptureWidget();

    void updateButtons();
    QPixmap pixmap();
    void showAppUpdateNotification(const QString& appLatestVersion,
                                   const QString& appLatestUrl);
    void setCaptureToolObjects(const CaptureToolObjects& captureToolObjects);

public slots:
    bool commitCurrentTool();
    void deleteToolWidgetOrClose();

signals:
    void captureTaken(uint id, QPixmap p, QRect selection);
    void captureFailed(uint id);
    void colorChanged(const QColor& c);
    void thicknessChanged(int thickness);

private slots:
    // TODO replace with tools
    void copyScreenshot();
    void saveScreenshot();
    void undo();
    void redo();
    void togglePanel();
    void childEnter();
    void childLeave();

    void selectAll();

    void resizeLeft();
    void resizeRight();
    void resizeUp();
    void resizeDown();

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void deleteCurrentTool();

    void setState(CaptureToolButton* b);
    void processTool(CaptureTool* t);
    void handleButtonSignal(CaptureTool::Request r);
    void setDrawColor(const QColor& c);
    void setDrawThickness(const int& t);
    void updateActiveLayer(const int& layer);

public:
    void removeToolObject(int index = -1);

protected:
    void paintEvent(QPaintEvent* paintEvent) override;
    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* keyEvent) override;
    void keyReleaseEvent(QKeyEvent* keyEvent) override;
    void wheelEvent(QWheelEvent* wheelEvent) override;
    void resizeEvent(QResizeEvent* resizeEvent) override;
    void moveEvent(QMoveEvent* moveEvent) override;

private:
    void loadDrawThickness();
    void pushObjectsStateToUndoStack();
    void releaseActiveTool();
    void uncheckActiveTool();
    int selectToolItemAtPos(const QPoint& pos);
    void showColorPicker(const QPoint& pos);
    bool startDrawObjectTool(const QPoint& pos);
    QPointer<CaptureTool> activeToolObject();
    void initContext(const QString& savePath, bool fullscreen);
    void initPanel();
    void initSelection();
    void initShortcuts();
    void updateSizeIndicator();
    void updateCursor();
    void pushToolToStack();
    void makeChild(QWidget* w);

    void repositionSelection(QRect r);
    void adjustSelection(QMargins m);
    void moveSelection(QPoint p);

    QRect extendedSelection() const;
    QRect extendedRect(QRect* r) const;
    void drawInitialMessage(QPainter* painter);
    void drawInactiveRegion(QPainter* painter);
    void drawToolsData(bool updateLayersPanel = true,
                       bool drawSelection = false);
    void drawObjectSelection();

    ////////////////////////////////////////
    // Class members

    // Context information
    CaptureContext m_context;

    // Main ui color
    QColor m_uiColor;
    // Secondary ui color
    QColor m_contrastUiColor;

    // Outside selection opacity
    int m_opacity;

    // utility flags
    bool m_mouseIsClicked;
    bool m_newSelection;
    bool m_grabbing;
    bool m_showInitialMsg;
    bool m_captureDone;
    bool m_previewEnabled;
    bool m_adjustmentButtonPressed;

    UpdateNotificationWidget* m_updateNotificationWidget;
    quint64 m_lastMouseWheel;
    QPointer<CaptureToolButton> m_sizeIndButton;
    // Last pressed button
    QPointer<CaptureToolButton> m_activeButton;
    QPointer<CaptureTool> m_activeTool;
    bool m_activeToolIsMoved;
    QPointer<QWidget> m_toolWidget;

    ButtonHandler* m_buttonHandler;
    UtilityPanel* m_panel;
    SidePanelWidget* m_sidePanel;
    ColorPicker* m_colorPicker;
    ConfigHandler m_config;
    NotifierBox* m_notifierBox;
    HoverEventFilter* m_eventFilter;
    SelectionWidget* m_selection;

    QPoint m_dragStartPoint;
    SelectionWidget::SideType m_mouseOverHandle;
    uint m_id;

    CaptureToolObjects m_captureToolObjects;
    CaptureToolObjects m_captureToolObjectsBackup;

    QPoint m_mousePressedPos;
    QPoint m_activeToolOffsetToMouseOnStart;

    QUndoStack m_undoStack;

    bool m_existingObjectIsChanged;

    // For start moving after more than X offset
    QPoint m_startMovePos;
    bool m_startMove;
};
