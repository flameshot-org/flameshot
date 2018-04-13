// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser <info@ckaiser.com.ar>
// released under the GNU GPL2  <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007 Luca Gugelmann <lucag@student.ethz.ch>
// released under the GNU LGPL  <http://www.gnu.org/licenses/old-licenses/library.txt>

#include "capturewidget.h"
#include "src/widgets/capture/hovereventfilter.h"
#include "src/utils/colorutils.h"
#include "src/utils/globalvalues.h"
#include "src/widgets/capture/notifierbox.h"
#include "src/widgets/capture/colorpicker.h"
#include "src/utils/screengrabber.h"
#include "src/utils/systemnotification.h"
#include "src/utils/screenshotsaver.h"
#include "src/core/controller.h"
#include "src/widgets/capture/modificationcommand.h"
#include <QUndoView>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QShortcut>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QBuffer>
#include <QDesktopWidget>

// CaptureWidget is the main component used to capture the screen. It contains an
// are of selection with its respective buttons.

// enableSaveWIndow
CaptureWidget::CaptureWidget(const uint id, const QString &savePath,
                             bool fullscreen, QWidget *parent) :
    QWidget(parent), m_mouseOverHandle(nullptr),
    m_mouseIsClicked(false), m_rightClick(false), m_newSelection(false),
    m_grabbing(false), m_captureDone(false), m_previewEnabled(true),
    m_activeButton(nullptr), m_activeTool(nullptr), m_id(id)
{
    m_eventFilter = new HoverEventFilter(this);
    connect(m_eventFilter, &HoverEventFilter::hoverIn,
            this, &CaptureWidget::childEnter);
    connect(m_eventFilter, &HoverEventFilter::hoverOut,
            this, &CaptureWidget::childLeave);

    initContext(savePath, fullscreen);
    initSelection();

    // Base config of the widget
    setAttribute(Qt::WA_DeleteOnClose);
    m_showInitialMsg = m_config.showHelpValue();
    m_opacity = m_config.contrastOpacityValue();
    setMouseTracking(true);
    updateCursor();
    initShortcuts();

#ifdef Q_OS_WIN
    // Top left of the whole set of screens
    QPoint topLeft(0,0);
#endif
    if (m_context.fullscreen) {
        // Grab Screenshot
        bool ok = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(ok);
        if(!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
            this->close();
        }
        m_context.origScreenshot = m_context.screenshot;

#ifdef Q_OS_WIN
        setWindowFlags(Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Popup);

        for (QScreen *const screen : QGuiApplication::screens()) {
            QPoint topLeftScreen = screen->geometry().topLeft();
            if (topLeft.x() > topLeftScreen.x() ||
                    topLeft.y() > topLeftScreen.y()) {
                topLeft = topLeftScreen;
            }
        }
        move(topLeft);
#else
        setWindowFlags(Qt::BypassWindowManagerHint
                       | Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Tool);
#endif
        resize(pixmap().size());
    }
    // Create buttons
    m_buttonHandler = new ButtonHandler(this);
    updateButtons();
    QVector<QRect> areas;
    if (m_context.fullscreen) {
        for (QScreen *const screen : QGuiApplication::screens()) {
            QRect r = screen->geometry();
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


    // Init color picker
    m_colorPicker = new ColorPicker(this);
    connect(m_colorPicker, &ColorPicker::colorSelected,
            this, &CaptureWidget::setDrawColor);
    m_colorPicker->hide();

    // Init notification widget
    m_notifierBox = new NotifierBox(this);
    m_notifierBox->hide();

    connect(&m_undoStack, &QUndoStack::indexChanged,
            this, [this](int){ this->update(); });
    initPanel();
}

CaptureWidget::~CaptureWidget() {
    if (m_captureDone) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        this->pixmap().save(&buffer, "PNG");
        emit captureTaken(m_id, byteArray);
    } else {
        emit captureFailed(m_id);
    }
    m_config.setdrawThickness(m_context.thickness);
}

// redefineButtons retrieves the buttons configured to be shown with the
// selection in the capture
void CaptureWidget::updateButtons() {
    m_uiColor = m_config.uiMainColorValue();
    m_contrastUiColor = m_config.uiContrastColorValue();

    auto buttons = m_config.getButtons();
    QVector<CaptureButton*> vectorButtons;

    for (const CaptureButton::ButtonType &t: buttons) {
        CaptureButton *b = new CaptureButton(t, this);
        if (t == CaptureButton::TYPE_SELECTIONINDICATOR) {
            m_sizeIndButton = b;
        }
        b->setColor(m_uiColor);
        makeChild(b);

        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
}

QPixmap CaptureWidget::pixmap() {
    return m_context.selectedScreenshotArea();
}

void CaptureWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_context.screenshot);

    if (m_activeTool && m_mouseIsClicked) {
        painter.save();
        m_activeTool->process(painter, m_context.screenshot);
        painter.restore();
    } else if (m_activeButton && m_activeButton->tool()->showMousePreview() &&
               m_previewEnabled)
    {
        painter.save();
        m_activeButton->tool()->paintMousePreview(painter, m_context);
        painter.restore();
    }

    QColor overlayColor(0, 0, 0, m_opacity);
    painter.setBrush(overlayColor);
    QRect r = m_context.selection.normalized().adjusted(0, 0, -1, -1);
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter.setClipRegion(grey);
    painter.drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter.setClipRect(rect());

    if (m_showInitialMsg) {
        QRect helpRect = QGuiApplication::primaryScreen()->geometry();
        helpRect.moveTo(mapFromGlobal(helpRect.topLeft()));

        QString helpTxt = tr("Select an area with the mouse, or press Esc to exit."
                             "\nPress Enter to capture the screen."
                             "\nPress Right Click to show the color picker."
                             "\nUse the Mouse Wheel to change the thickness of your tool.");

        // We draw the white contrasting background for the text, using the
        //same text and options to get the boundingRect that the text will have.
        QRectF bRect = painter.boundingRect(helpRect, Qt::AlignCenter, helpTxt);

        // These four calls provide padding for the rect
        const int margin = QApplication::fontMetrics().height() / 2;
        bRect.setWidth(bRect.width() + margin);
        bRect.setHeight(bRect.height() + margin);
        bRect.setX(bRect.x() - margin);
        bRect.setY(bRect.y() - margin);

        QColor rectColor(m_uiColor);
        rectColor.setAlpha(180);
        QColor textColor((ColorUtils::colorIsDark(rectColor) ?
                              Qt::white : Qt::black));

        painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
        painter.setPen(QPen(textColor));

        painter.drawRect(bRect);
        painter.drawText(helpRect, Qt::AlignCenter, helpTxt);
    }

    if (!m_context.selection.isNull()) {
        // paint selection rect
        painter.setPen(m_uiColor);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r);

        // paint handlers
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(m_uiColor);
        for(auto r: m_handles) {
            painter.drawRoundRect(*r, 100, 100);
        }
    }
}

void CaptureWidget::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_rightClick = true;
        m_colorPicker->move(e->pos().x()-m_colorPicker->width()/2,
                            e->pos().y()-m_colorPicker->height()/2);
        m_colorPicker->show();
    } else if (e->button() == Qt::LeftButton) {
        m_showInitialMsg = false;
        m_mouseIsClicked = true;
        if (m_activeButton) {
            if (m_activeTool) {
                m_activeTool->deleteLater();
            }
            m_activeTool = m_activeButton->tool()->copy(this);
            connect(m_activeTool, &CaptureTool::requestAction,
                    this, &CaptureWidget::handleButtonSignal);
            m_activeTool->drawStart(m_context);
            return;
        }

        m_dragStartPoint = e->pos();
        m_selectionBeforeDrag = m_context.selection;
        if (!m_context.selection.contains(e->pos()) && !m_mouseOverHandle) {
            m_newSelection = true;
            m_context.selection = QRect();
            m_buttonHandler->hide();
            update();
        } else {
            m_grabbing = true;
        }
    }
    updateCursor();
}

void CaptureWidget::mouseMoveEvent(QMouseEvent *e) {
    m_context.mousePos = e->pos();

    if (m_mouseIsClicked && !m_activeButton) {
        if (m_buttonHandler->isVisible()) {
            m_buttonHandler->hide();
        }
        if (m_newSelection) {
            m_context.selection =
                    QRect(m_dragStartPoint, m_context.mousePos).normalized();
            updateHandles();
            update();
        } else if (!m_mouseOverHandle) {
            // Moving the whole selection
            QRect r = rect().normalized();
            QRect initialRect = m_context.selection.normalized();
            // new top left
            QPoint p = initialRect.topLeft() + (e->pos() - m_dragStartPoint);
            m_dragStartPoint += e->pos() - m_dragStartPoint;
            m_context.selection.moveTo(p);
            if (!r.contains(QPoint(r.center().x(), m_context.selection.top()))) {
                m_context.selection.setTop(r.top());
            } if (!r.contains(QPoint(m_context.selection.left(), r.center().y()))) {
                m_context.selection.setLeft(r.left());
            }
            if (!r.contains(QPoint(m_context.selection.right(), r.center().y()))) {
                m_context.selection.setRight(r.right());
            } if (!r.contains(QPoint(r.center().x(), m_context.selection.bottom()))) {
                m_context.selection.setBottom(r.bottom());
            }
            updateHandles();
            update();
        } else {
            // Dragging a handle
            QRect r = m_selectionBeforeDrag;
            QPoint offset = e->pos() - m_dragStartPoint;
            bool symmetryMod = qApp->keyboardModifiers() & Qt::ShiftModifier;

            if (m_mouseOverHandle == &m_TLHandle || m_mouseOverHandle == &m_TSide
                    || m_mouseOverHandle == &m_TRHandle)
            { // dragging one of the top handles
                r.setTop(r.top() + offset.y());
                if (symmetryMod) {
                    r.setBottom(r.bottom() - offset.y());
                }
            }
            if (m_mouseOverHandle == &m_TLHandle || m_mouseOverHandle == &m_LSide
                    || m_mouseOverHandle == &m_BLHandle)
            { // dragging one of the left handles
                r.setLeft(r.left() + offset.x());
                if (symmetryMod) {
                    r.setRight(r.right() - offset.x());
                }
            }
            if (m_mouseOverHandle == &m_BLHandle || m_mouseOverHandle == &m_BSide
                    || m_mouseOverHandle == &m_BRHandle)
            { // dragging one of the bottom handles
                r.setBottom(r.bottom() + offset.y());
                if (symmetryMod) {
                    r.setTop(r.top() - offset.y());
                }
            }
            if (m_mouseOverHandle == &m_TRHandle || m_mouseOverHandle == &m_RSide
                    || m_mouseOverHandle == &m_BRHandle)
            { // dragging one of the right handles
                r.setRight(r.right() + offset.x());
                if (symmetryMod) {
                    r.setLeft(r.left() - offset.x());
                }
            }
            m_context.selection = r.normalized();
            updateHandles();
            update();
        }
    } else if (m_mouseIsClicked && m_activeTool) {
        // drawing with a tool
        m_activeTool->drawMove(e->pos());
        update();
        // Hides the buttons under the mouse. If the mouse leaves, it shows them.
        if (m_buttonHandler->buttonsAreInside()) {
            const bool containsMouse = m_buttonHandler->contains(m_context.mousePos);
            if (containsMouse) {
                m_buttonHandler->hide();
            } else {
                m_buttonHandler->show();
            }
        }
    } else if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    } else {
        if (m_context.selection.isNull()) {
            return;
        }
        bool found = false;
        for (QRect *const r: m_sides) {
            if (r->contains(e->pos())) {
                m_mouseOverHandle = r;
                found = true;
                break;
            }
        }
        if (!found) {
            m_mouseOverHandle = nullptr;
        }
        updateCursor();
    }
}

void CaptureWidget::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_colorPicker->hide();
        m_rightClick = false;
    // when we end the drawing we have to register the last  point and
    //add the temp modification to the list of modifications
    } else if (m_mouseIsClicked && m_activeTool) {
        m_activeTool->drawEnd(m_context.mousePos);
        if (m_activeTool->isValid()) {
            auto mod = new ModificationCommand(
                        &m_context.screenshot, m_activeTool);
            m_undoStack.push(mod);
        }
        m_activeTool = nullptr;
    }

    // Show the buttons after the resize of the selection or the creation
    // of a new one.
    if (!m_buttonHandler->isVisible() && !m_context.selection.isNull()) {
        // Don't go outside
        m_context.selection = m_context.selection.intersected(rect());
        updateSizeIndicator();
        m_buttonHandler->updatePosition(m_context.selection);
        m_buttonHandler->show();
    }
    m_mouseIsClicked = false;
    m_newSelection = false;
    m_grabbing = false;

    updateCursor();
}

void CaptureWidget::keyPressEvent(QKeyEvent *e) {
    if (m_context.selection.isNull()) {
        return;
    } else if (e->key() == Qt::Key_Up
               && m_context.selection.top() > rect().top()) {
        m_context.selection.moveTop(m_context.selection.top()-1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateHandles();
        update();
    } else if (e->key() == Qt::Key_Down
               && m_context.selection.bottom() < rect().bottom()) {
        m_context.selection.moveBottom(m_context.selection.bottom()+1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateHandles();
        update();
    } else if (e->key() == Qt::Key_Left
               && m_context.selection.left() > rect().left()) {
        m_context.selection.moveLeft(m_context.selection.left()-1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateHandles();
        update();
    } else if (e->key() == Qt::Key_Right
               && m_context.selection.right() < rect().right()) {
        m_context.selection.moveRight(m_context.selection.right()+1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateHandles();
        update();
    }
}

void CaptureWidget::wheelEvent(QWheelEvent *e) {
    m_context.thickness += e->delta() / 120;
    m_context.thickness = qBound(0, m_context.thickness, 100);
    QPoint topLeft = qApp->desktop()->screenGeometry(
                qApp->desktop()->screenNumber(QCursor::pos())).topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_context.thickness));
    if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    }
}

void CaptureWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    m_context.widgetDimensions = rect();
    m_context.widgetOffset = mapToGlobal(QPoint(0,0));
}

void CaptureWidget::moveEvent(QMoveEvent *e) {
    QWidget::moveEvent(e);
    m_context.widgetOffset = mapToGlobal(QPoint(0,0));
}

void CaptureWidget::initContext(const QString &savePath, bool fullscreen) {
    m_context.widgetDimensions = rect();
    m_context.color = m_config.drawColorValue();
    m_context.savePath = savePath;
    m_context.widgetOffset = mapToGlobal(QPoint(0,0));
    m_context.mousePos= mapFromGlobal(QCursor::pos());
    m_context.thickness = m_config.drawThicknessValue();
    m_context.fullscreen = fullscreen;
}

void CaptureWidget::initPanel() {
    m_panel = new UtilityPanel(this);
    makeChild(m_panel);
    QRect panelRect = QGuiApplication::primaryScreen()->geometry();
    panelRect.moveTo(mapFromGlobal(panelRect.topLeft()));
    panelRect.setWidth(m_colorPicker->width() * 3);
    m_panel->setGeometry(panelRect);

    m_panel->pushWidget(new QUndoView(&m_undoStack, this));
}

void CaptureWidget::initSelection() {
    QRect baseRect(0, 0, handleSize(), handleSize());
    m_TLHandle = baseRect; m_TRHandle = baseRect;
    m_BLHandle = baseRect; m_BRHandle = baseRect;
    m_LHandle = baseRect; m_THandle = baseRect;
    m_RHandle = baseRect; m_BHandle = baseRect;

    m_handles << &m_TLHandle << &m_TRHandle << &m_BLHandle << &m_BRHandle
    << &m_LHandle << &m_THandle << &m_RHandle << &m_BHandle;

    m_sides << &m_TLHandle << &m_TRHandle << &m_BLHandle << &m_BRHandle
            << &m_LSide << &m_TSide << &m_RSide << &m_BSide;
}

void CaptureWidget::initWidget() {

}

void CaptureWidget::setState(CaptureButton *b) {
    if (!b) {
        return;
    }
    processTool(b->tool());
    // Only close activated from button
    if (b->tool()->closeOnButtonPressed()) {
        close();
    }

    if (b->tool()->isSelectable()) {
        if (m_activeButton != b) {
            if (m_activeButton) {
                m_activeButton->setColor(m_uiColor);
            }
            m_activeButton = b;
            m_activeButton->setColor(m_contrastUiColor);
        } else if (m_activeButton) {
            m_activeButton->setColor(m_uiColor);
            m_activeButton = nullptr;
        }
        update(); // clear mouse preview
    }
}

void CaptureWidget::processTool(CaptureTool *t) {
    auto backup = m_activeTool;
    // The tool is active during the pressed().
    m_activeTool = t;
    t->pressed(m_context);
    m_activeTool = backup;
    QWidget *cw = t->configurationWidget();
    if (cw) {
        m_panel->addToolWidget(t->configurationWidget());
    }
}

void CaptureWidget::handleButtonSignal(CaptureTool::Request r) {
    switch (r) {
    case CaptureTool::REQ_CLEAR_MODIFICATIONS:
        m_undoStack.clear(); // TODO
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
        m_context.selection = QRect();
        updateCursor();
        break;
    case CaptureTool::REQ_SELECT_ALL:
        m_context.selection = rect();
        break;
    case CaptureTool::REQ_UNDO_MODIFICATION:
        m_undoStack.undo();
        break;
    case CaptureTool::REQ_REDO_MODIFICATION:
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
    case CaptureTool::REQ_ADD_CHILD_WIDGETS:
        if (m_activeTool) {
            QWidget *w = m_activeTool->widget();
            makeChild(w);
            w->move(m_context.mousePos);
            w->show();
        }
        break;
    case CaptureTool::REQ_ADD_CHILD_WINDOW:
        if (m_activeTool) {
            QWidget *w = m_activeTool->widget();
            connect(this, &CaptureWidget::destroyed, w, &QWidget::deleteLater);
            w->show();
        }
        break;
    case CaptureTool::REQ_ADD_EXTERNAL_WIDGETS:
        if (m_activeTool) {
            QWidget *w = m_activeTool->widget();
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
        }
        break;
    default:
        break;
    }
}

void CaptureWidget::setDrawColor(const QColor &c) {
    m_context.color = c;
    ConfigHandler().setDrawColor(m_context.color);
}

void CaptureWidget::leftResize() {
    if (!m_context.selection.isNull() && m_context.selection.right() > m_context.selection.left()) {
        m_context.selection.setRight(m_context.selection.right()-1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::rightResize() {
    if (!m_context.selection.isNull() && m_context.selection.right() < rect().right()) {
        m_context.selection.setRight(m_context.selection.right()+1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::upResize() {
    if (!m_context.selection.isNull() && m_context.selection.bottom() > m_context.selection.top()) {
        m_context.selection.setBottom(m_context.selection.bottom()-1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::downResize() {
    if (!m_context.selection.isNull() && m_context.selection.bottom() < rect().bottom()) {
        m_context.selection.setBottom(m_context.selection.bottom()+1);
        m_buttonHandler->updatePosition(m_context.selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::initShortcuts() {
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(saveScreenshot()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this, SLOT(copyScreenshot()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(undo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z), this, SLOT(redo()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this, SLOT(rightResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this, SLOT(leftResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up), this, SLOT(upResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down), this, SLOT(downResize()));
    new QShortcut(Qt::Key_Space, this, SLOT(togglePanel()));
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
    new QShortcut(Qt::Key_Return, this, SLOT(copyScreenshot()));
}

void CaptureWidget::updateHandles() {
    QRect r = m_context.selection.normalized().adjusted(0, 0, -1, -1);
    int s2 = handleSize() / 2;

    m_TLHandle.moveTopLeft(QPoint(r.x() - s2, r.y() - s2));
    m_TRHandle.moveTopRight(QPoint(r.right() + s2, r.y() - s2));
    m_BRHandle.moveBottomRight(QPoint(r.x() + r.width() + s2, r.bottom() + s2));
    m_BLHandle.moveBottomLeft(QPoint(QPoint(r.x() - s2, r.bottom() + s2)));

    m_LHandle.moveTopLeft(QPoint(r.x() - s2, r.y() + r.height() / 2 - s2));
    m_THandle.moveTopLeft(QPoint(r.x() + r.width() / 2 - s2, r.y() - s2));
    m_RHandle.moveTopRight(QPoint(r.right() + s2, r.y() + r.height() / 2 - s2));
    m_BHandle.moveBottomLeft(QPoint(r.x() + r.width() / 2 - s2, r.bottom() + s2));

    m_LSide = QRect(m_TLHandle.bottomLeft(), m_BLHandle.topRight());
    m_RSide = QRect(m_TRHandle.bottomLeft(), m_BRHandle.topRight());
    m_BSide = QRect(m_BLHandle.topRight(), m_BRHandle.bottomLeft());
    m_TSide = QRect(m_TLHandle.topRight(), m_TRHandle.bottomLeft());
}

void CaptureWidget::updateSizeIndicator() {
    if (m_sizeIndButton){
        const QRect &selection = extendedSelection();
        m_sizeIndButton->setText(QStringLiteral("%1\n%2")
                                     .arg(selection.width())
                                     .arg(selection.height()));
    }
}

void CaptureWidget::updateCursor() {
    if (m_rightClick) {
        setCursor(Qt::ArrowCursor);
    } else if (m_grabbing) {
        setCursor(Qt::ClosedHandCursor);
    } else if (!m_activeButton) {
        if (m_mouseOverHandle){
            // cursor on the handlers
            if (m_mouseOverHandle == &m_TLHandle || m_mouseOverHandle == &m_BRHandle) {
                setCursor(Qt::SizeFDiagCursor);
            } else if (m_mouseOverHandle == &m_TRHandle || m_mouseOverHandle == &m_BLHandle) {
                setCursor(Qt::SizeBDiagCursor);
            } else if (m_mouseOverHandle == &m_LSide || m_mouseOverHandle == &m_RSide) {
                setCursor(Qt::SizeHorCursor);
            } else if (m_mouseOverHandle == &m_TSide || m_mouseOverHandle == &m_BSide) {
                setCursor(Qt::SizeVerCursor);
            }
        } else if (m_context.selection.contains(m_context.mousePos)) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
    } else {
        setCursor(Qt::CrossCursor);
    }
}

void CaptureWidget::makeChild(QWidget *w) {
    w->setParent(this);
    w->installEventFilter(m_eventFilter);
}

void CaptureWidget::togglePanel() {
    m_panel->toggle();
}

void CaptureWidget::childEnter() {
    m_previewEnabled = false;
    update();
}

void CaptureWidget::childLeave() {
    m_previewEnabled = true;
    update();
}

int CaptureWidget::handleSize() {
    return (QApplication::fontMetrics().height() * 0.7);
}

void CaptureWidget::copyScreenshot() {
    m_captureDone = true;
    ScreenshotSaver().saveToClipboard(m_context.selectedScreenshotArea());
    close();
}

void CaptureWidget::saveScreenshot() {
    m_captureDone = true;
    hide();
    if (m_context.savePath.isEmpty()) {
        ScreenshotSaver().saveToFilesystemGUI(pixmap());
    } else {
        ScreenshotSaver().saveToFilesystem(pixmap(), m_context.savePath);
    }
    close();
}

void CaptureWidget::undo() {
    m_undoStack.undo();
}

void CaptureWidget::redo() {
    m_undoStack.redo();
}

QRect CaptureWidget::extendedSelection() const {
    if (m_context.selection.isNull())
        return QRect();
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();

    QRect const &r = m_context.selection;
    return QRect(r.left()   * devicePixelRatio,
                 r.top()    * devicePixelRatio,
                 r.width()  * devicePixelRatio,
                 r.height() * devicePixelRatio);
}
