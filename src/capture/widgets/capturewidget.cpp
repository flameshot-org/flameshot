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

#include "src/capture/screenshot.h"
#include "src/capture/capturemodification.h"
#include "capturewidget.h"
#include "capturebutton.h"
#include "src/capture/widgets/notifierbox.h"
#include "src/capture/widgets/colorpicker.h"
#include "src/utils/screengrabber.h"
#include "src/utils/systemnotification.h"
#include "src/core/resourceexporter.h"
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
CaptureWidget::CaptureWidget(const uint id, const QString &forcedSavePath,
                             CaptureWidget::LaunchMode mode, QWidget *parent) :
    QWidget(parent), m_screenshot(nullptr), m_mouseOverHandle(0),
    m_mouseIsClicked(false), m_rightClick(false), m_newSelection(false),
    m_grabbing(false), m_captureDone(false), m_toolIsForDrawing(false),
    m_forcedSavePath(forcedSavePath), m_id(id),
    m_state(CaptureButton::TYPE_MOVESELECTION)
{
    m_showInitialMsg = m_config.showHelpValue();
    m_thickness = m_config.drawThicknessValue();
    m_opacity = m_config.contrastOpacityValue();

    setAttribute(Qt::WA_DeleteOnClose);
    // create selection handlers

    QRect baseRect(0, 0, handleSize(), handleSize());
    m_TLHandle = baseRect; m_TRHandle = baseRect;
    m_BLHandle = baseRect; m_BRHandle = baseRect;
    m_LHandle = baseRect; m_THandle = baseRect;
    m_RHandle = baseRect; m_BHandle = baseRect;

    m_handles << &m_TLHandle << &m_TRHandle << &m_BLHandle << &m_BRHandle
    << &m_LHandle << &m_THandle << &m_RHandle << &m_BHandle;

    m_sides << &m_TLHandle << &m_TRHandle << &m_BLHandle << &m_BRHandle
            << &m_LSide << &m_TSide << &m_RSide << &m_BSide;

    // set base config of the widget
    setMouseTracking(true);
    updateCursor();
    initShortcuts();

#ifdef Q_OS_WIN
    QPoint topLeft(0,0);
#endif
    if (mode == FULLSCREEN) {
        // init content
        bool ok = true;
        QPixmap fullScreenshot(ScreenGrabber().grabEntireDesktop(ok));
        if(!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
            this->close();
        }
        m_screenshot = new Screenshot(fullScreenshot, this);

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
    // create buttons
    m_buttonHandler = new ButtonHandler(this);
    updateButtons();
    QVector<QRect> areas;
    if (mode == FULLSCREEN) {
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
    // init interface color
    m_colorPicker = new ColorPicker(this);
    m_colorPicker->hide();

    m_notifierBox = new NotifierBox(this);
    m_notifierBox->hide();
}

CaptureWidget::~CaptureWidget() {
    if (m_captureDone) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        this->pixmap().save(&buffer, "PNG");
        Q_EMIT captureTaken(m_id, byteArray);
    } else {
        Q_EMIT captureFailed(m_id);
    }
    m_config.setdrawThickness(m_thickness);
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

        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
}

QPixmap CaptureWidget::pixmap() {
    if (m_selection.isNull()) { // copy full screen when no selection
        return m_screenshot->screenshot();
    } else {
        return m_screenshot->screenshot().copy(extendedSelection());
    }
}

void CaptureWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // If we are creating a new modification to the screenshot we just draw
    // a temporal modification without antialiasing in the pencil tool for
    // performance. When we are not drawing we just shot the modified screenshot
    bool stateIsSelection = m_state == CaptureButton::TYPE_MOVESELECTION;
    if (m_mouseIsClicked && !stateIsSelection) {
        painter.drawPixmap(0, 0, m_screenshot->paintTemporalModification(
                               m_modifications.last()));
    } else if (m_toolIsForDrawing && !stateIsSelection) {
        CaptureButton::ButtonType type = CaptureButton::ButtonType::TYPE_LINE;
        if (m_state == CaptureButton::ButtonType::TYPE_MARKER) {
            type = CaptureButton::ButtonType::TYPE_MARKER;
        }
        CaptureModification tempMod(type, m_mousePos,
                                    m_colorPicker->drawColor(),
                                    m_thickness);
        tempMod.addPoint(m_mousePos);
        painter.drawPixmap(0, 0, m_screenshot->paintTemporalModification(
                               &tempMod));
    } else {
        painter.drawPixmap(0, 0, m_screenshot->screenshot());
    }

    QColor overlayColor(0, 0, 0, m_opacity);
    painter.setBrush(overlayColor);
    QRect r = m_selection.normalized().adjusted(0, 0, -1, -1);
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
        QColor textColor((CaptureButton::iconIsWhiteByColor(rectColor) ?
                              Qt::white : Qt::black));

        painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
        painter.setPen(QPen(textColor));

        painter.drawRect(bRect);
        painter.drawText(helpRect, Qt::AlignCenter, helpTxt);
    }

    if (!m_selection.isNull()) {
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
        if (m_state != CaptureButton::TYPE_MOVESELECTION) {
            auto mod = new CaptureModification(m_state, e->pos(),
                                               m_colorPicker->drawColor(),
                                               m_thickness,
                                               this);
            m_modifications.append(mod);
            return;
        }
        m_dragStartPoint = e->pos();
        m_selectionBeforeDrag = m_selection;
        if (!m_selection.contains(e->pos()) && !m_mouseOverHandle) {
            m_newSelection = true;
            m_selection = QRect();
            m_buttonHandler->hide();
            update();
        } else {
            m_grabbing = true;
        }
    }
    updateCursor();
}

void CaptureWidget::mouseMoveEvent(QMouseEvent *e) {
    m_mousePos = e->pos();

    if (m_mouseIsClicked && m_state == CaptureButton::TYPE_MOVESELECTION) {
        if (m_buttonHandler->isVisible()) {
            m_buttonHandler->hide();
        }
        if (m_newSelection) {
            m_selection = QRect(m_dragStartPoint, m_mousePos).normalized();
            updateHandles();
            update();
        } else if (!m_mouseOverHandle) {
            // Moving the whole selection
            QRect r = rect().normalized();
            QRect initialRect = m_selection.normalized();
            // new top left
            QPoint p = initialRect.topLeft() + (e->pos() - m_dragStartPoint);
            m_dragStartPoint += e->pos() - m_dragStartPoint;
            m_selection.moveTo(p);
            if (!r.contains(QPoint(r.center().x(), m_selection.top()))) {
                m_selection.setTop(r.top());
            } if (!r.contains(QPoint(m_selection.left(), r.center().y()))) {
                m_selection.setLeft(r.left());
            }
            if (!r.contains(QPoint(m_selection.right(), r.center().y()))) {
                m_selection.setRight(r.right());
            } if (!r.contains(QPoint(r.center().x(), m_selection.bottom()))) {
                m_selection.setBottom(r.bottom());
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
            m_selection = r.normalized();
            updateHandles();
            update();
        }
    } else if (m_mouseIsClicked && m_state != CaptureButton::TYPE_MOVESELECTION) {
        // drawing with a tool
        m_modifications.last()->addPoint(e->pos());
        update();
        // Hides the buttons under the mouse. If the mouse leaves, it shows them.
        if (m_buttonHandler->buttonsAreInside()) {
            const bool containsMouse = m_buttonHandler->contains(m_mousePos);
            if (containsMouse) {
                m_buttonHandler->hide();
            } else {
                m_buttonHandler->show();
            }
        }
    } else if (m_toolIsForDrawing) {
        update();
    } else {
        if (m_selection.isNull()) {
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
    // when we end the drawing of a modification in the capture we have to
    // register the last point and add the whole modification to the screenshot
    } else if (m_mouseIsClicked && m_state != CaptureButton::TYPE_MOVESELECTION) {
        m_screenshot->paintModification(m_modifications.last());
        update();
    }

    if (!m_buttonHandler->isVisible() && !m_selection.isNull()) {
        updateSizeIndicator();
        m_buttonHandler->updatePosition(m_selection);
        m_buttonHandler->show();
    }
    m_mouseIsClicked = false;
    m_newSelection = false;
    m_grabbing = false;

    updateCursor();
}

void CaptureWidget::keyPressEvent(QKeyEvent *e) {
    if (m_selection.isNull()) {
        return;
    } else if (e->key() == Qt::Key_Up
               && m_selection.top() > rect().top()) {
        m_selection.moveTop(m_selection.top()-1);
        m_buttonHandler->updatePosition(m_selection);
        updateHandles();
        update();
    } else if (e->key() == Qt::Key_Down
               && m_selection.bottom() < rect().bottom()) {
        m_selection.moveBottom(m_selection.bottom()+1);
        m_buttonHandler->updatePosition(m_selection);
        updateHandles();
        update();
    } else if (e->key() == Qt::Key_Left
               && m_selection.left() > rect().left()) {
        m_selection.moveLeft(m_selection.left()-1);
        m_buttonHandler->updatePosition(m_selection);
        updateHandles();
        update();
    } else if (e->key() == Qt::Key_Right
               && m_selection.right() < rect().right()) {
        m_selection.moveRight(m_selection.right()+1);
        m_buttonHandler->updatePosition(m_selection);
        updateHandles();
        update();
    }
}

void CaptureWidget::wheelEvent(QWheelEvent *e) {
    m_thickness += e->delta() / 120;
    m_thickness = qBound(0, m_thickness, 100);
    QPoint topLeft = qApp->desktop()->screenGeometry(
                qApp->desktop()->screenNumber(QCursor::pos())).topLeft();
    int offset = m_notifierBox->width() / 4;
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
    m_notifierBox->showMessage(QString::number(m_thickness));
    if (m_toolIsForDrawing) {
        update();
    }
}

bool CaptureWidget::undo() {
    bool itemRemoved = false;
    if (!m_modifications.isEmpty()) {
        m_modifications.last()->deleteLater();
        m_modifications.pop_back();
        m_screenshot->overrideModifications(m_modifications);
        update();
        itemRemoved = true;
    }
    return itemRemoved;
}

void CaptureWidget::setState(CaptureButton *b) {
    CaptureButton::ButtonType t = b->buttonType();
    if (b->tool()->isSelectable()) {
        if (t != m_state) {
            m_state = t;
            m_toolIsForDrawing =
                    (b->tool()->toolType() !=
                    CaptureTool::ToolWorkType::TYPE_WORKER) &&
                    b->buttonType() != CaptureButton::TYPE_BLUR;
            if (m_lastPressedButton) {
                m_lastPressedButton->setColor(m_uiColor);
            }
            m_lastPressedButton = b;
            m_lastPressedButton->setColor(m_contrastUiColor);
        } else {
            handleButtonSignal(CaptureTool::REQ_MOVE_MODE);
        }
        update(); // clear mouse preview
    }
}

void CaptureWidget::handleButtonSignal(CaptureTool::Request r) {
    switch (r) {
    case CaptureTool::REQ_CLEAR_MODIFICATIONS:
        while(undo());
        break;
    case CaptureTool::REQ_CLOSE_GUI:
        close();
        break;
    case CaptureTool::REQ_HIDE_GUI:
        hide();
        break;
    case CaptureTool::REQ_HIDE_SELECTION:
        m_newSelection = true;
        m_selection = QRect();
        updateCursor();
        break;
    case CaptureTool::REQ_SAVE_SCREENSHOT:
        saveScreenshot();
        break;
    case CaptureTool::REQ_SELECT_ALL:
        m_selection = rect();
        break;
    case CaptureTool::REQ_TO_CLIPBOARD:
        copyScreenshot();
        break;
    case CaptureTool::REQ_UNDO_MODIFICATION:
        undo();
        break;
    case CaptureTool::REQ_UPLOAD_TO_IMGUR:
        uploadToImgur();
        break;
    case CaptureTool::REQ_OPEN_APP:
        openWithProgram();
        break;
    case CaptureTool::REQ_MOVE_MODE:
        m_state = CaptureButton::TYPE_MOVESELECTION;
        m_toolIsForDrawing = false;
        if (m_lastPressedButton) {
            m_lastPressedButton->setColor(m_uiColor);
            m_lastPressedButton = nullptr;
        }
        break;
    default:
        break;
    }
}

void CaptureWidget::leftResize() {
    if (!m_selection.isNull() && m_selection.right() > m_selection.left()) {
        m_selection.setRight(m_selection.right()-1);
        m_buttonHandler->updatePosition(m_selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::rightResize() {
    if (!m_selection.isNull() && m_selection.right() < rect().right()) {
        m_selection.setRight(m_selection.right()+1);
        m_buttonHandler->updatePosition(m_selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::upResize() {
    if (!m_selection.isNull() && m_selection.bottom() > m_selection.top()) {
        m_selection.setBottom(m_selection.bottom()-1);
        m_buttonHandler->updatePosition(m_selection);
        updateSizeIndicator();
        updateHandles();
        update();
    }
}

void CaptureWidget::downResize() {
    if (!m_selection.isNull() && m_selection.bottom() < rect().bottom()) {
        m_selection.setBottom(m_selection.bottom()+1);
        m_buttonHandler->updatePosition(m_selection);
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
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this, SLOT(rightResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this, SLOT(leftResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up), this, SLOT(upResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down), this, SLOT(downResize()));
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
    new QShortcut(Qt::Key_Return, this, SLOT(copyScreenshot()));
}

void CaptureWidget::updateHandles() {
    QRect r = m_selection.normalized().adjusted(0, 0, -1, -1);
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
    } else if (m_state == CaptureButton::TYPE_MOVESELECTION) {
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
        } else if (m_selection.contains(m_mousePos)) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
    } else {
        setCursor(Qt::CrossCursor);
    }

}

int CaptureWidget::handleSize() {
    return (QApplication::fontMetrics().height() * 0.7);
}

void CaptureWidget::copyScreenshot() {
    m_captureDone = true;
    hide();
    ResourceExporter().captureToClipboard(pixmap());
    close();
}

void CaptureWidget::saveScreenshot() {
    m_captureDone = true;
    hide();
    if (m_forcedSavePath.isEmpty()) {
        ResourceExporter().captureToFileUi(pixmap());
    } else {
        ResourceExporter().captureToFile(pixmap(), m_forcedSavePath);
    }
    close();
}

void CaptureWidget::uploadToImgur() {
    m_captureDone = true;
    hide();
    ResourceExporter().captureToImgur(pixmap());
    close();
}

void CaptureWidget::openWithProgram() {
    m_captureDone = true;
    hide();
    ResourceExporter().captureToProgram(pixmap());
    close();
}

QRect CaptureWidget::extendedSelection() const {
    if (m_selection.isNull())
        return QRect();
    auto devicePixelRatio = m_screenshot->screenshot().devicePixelRatio();

    return QRect(m_selection.left()   * devicePixelRatio,
                 m_selection.top()    * devicePixelRatio,
                 m_selection.width()  * devicePixelRatio,
                 m_selection.height() * devicePixelRatio);
}
