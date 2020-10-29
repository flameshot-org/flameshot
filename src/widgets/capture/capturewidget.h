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

// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser
// <info@ckaiser.com.ar> released under the GNU GPL2
// <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007
// Luca Gugelmann <lucag@student.ethz.ch> released under the GNU LGPL
// <http://www.gnu.org/licenses/old-licenses/library.txt>

#pragma once

#include "buttonhandler.h"
#include "capturetoolbutton.h"
#include "src/tools/capturecontext.h"
#include "src/tools/capturetool.h"
#include "src/utils/confighandler.h"
#include "src/widgets/capture/selectionwidget.h"
#include "src/widgets/panel/utilitypanel.h"
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

class CaptureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureWidget(const uint id = 0,
                           const QString& savePath = QString(),
                           bool fullScreen = true,
                           QWidget* parent = nullptr);
    ~CaptureWidget();

    void updateButtons();
    QPixmap pixmap();

public slots:
    bool commitCurrentTool();
    void deleteToolwidgetOrClose();

signals:
    void captureTaken(uint id, QPixmap p, QRect selection);
    void captureFailed(uint id);
    void colorChanged(const QColor& c);
    void thicknessChanged(const int thickness);

private slots:

    // TODO replace with tools
    void copyScreenshot();
    void saveScreenshot();
    void undo();
    void redo();
    void togglePanel();
    void childEnter();
    void childLeave();

    void resizeLeft();
    void resizeRight();
    void resizeUp();
    void resizeDown();

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void setState(CaptureToolButton* b);
    void processTool(CaptureTool* t);
    void handleButtonSignal(CaptureTool::Request r);
    void setDrawColor(const QColor& c);
    void setDrawThickness(const int& t);
    void incrementCircleCount();
    void decrementCircleCount();

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent*);
    void resizeEvent(QResizeEvent*);
    void moveEvent(QMoveEvent*);

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
    bool m_rightClick;
    bool m_newSelection;
    bool m_grabbing;
    bool m_showInitialMsg;
    bool m_captureDone;
    bool m_previewEnabled;
    bool m_adjustmentButtonPressed;

private:
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

private:
    QRect extendedSelection() const;
    QRect extendedRect(QRect* r) const;

private:
    QUndoStack m_undoStack;
    QPointer<CaptureToolButton> m_sizeIndButton;
    // Last pressed button
    QPointer<CaptureToolButton> m_activeButton;
    QPointer<CaptureTool> m_activeTool;
    QPointer<QWidget> m_toolWidget;

    ButtonHandler* m_buttonHandler;
    UtilityPanel* m_panel;
    ColorPicker* m_colorPicker;
    ConfigHandler m_config;
    NotifierBox* m_notifierBox;
    HoverEventFilter* m_eventFilter;
    SelectionWidget* m_selection;

    QPoint m_dragStartPoint;
    SelectionWidget::SideType m_mouseOverHandle;
    uint m_id;
};
