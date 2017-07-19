// Copyright 2017 Alejandro Sirgo Rica
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

#include "screenshot.h"
#include "capturemodification.h"
#include "capturewidget.h"
#include "capturebutton.h"
#include "src/capture/colorpicker.h"
#include "src/capture/screengrabber.h"
#include "src/utils/confighandler.h"
#include <QScreen>
#include <QWindow>
#include <QGuiApplication>
#include <QApplication>
#include <QScreen>
#include <QShortcut>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDesktopServices>

// CaptureWidget is the main component used to capture the screen. It contains an
// are of selection with its respective buttons.

namespace {
    // size of the handlers at the corners of the selection
    const int HANDLE_SIZE = 9;
}

// http://doc.qt.io/qt-5/qwidget.html#setMask

CaptureWidget::CaptureWidget(bool enableSaveWindow, QWidget *parent) :
    QWidget(parent), m_mouseOverHandle(0), m_mouseIsClicked(false),
    m_rightClick(false), m_newSelection(false), m_grabbing(false),
    m_onButton(false), m_enableSaveWindow(enableSaveWindow),
    m_state(CaptureButton::TYPE_MOVESELECTION)
{
    m_showInitialMsg = ConfigHandler().getShowHelp();

    setAttribute(Qt::WA_DeleteOnClose);
    // create selection handlers
    QRect baseRect(0, 0, HANDLE_SIZE, HANDLE_SIZE);
    m_TLHandle = baseRect; m_TRHandle = baseRect;
    m_BLHandle = baseRect; m_BRHandle = baseRect;
    m_LHandle = baseRect; m_THandle = baseRect;
    m_RHandle = baseRect; m_BHandle = baseRect;

    m_Handles << &m_TLHandle << &m_TRHandle << &m_BLHandle << &m_BRHandle
    << &m_LHandle << &m_THandle << &m_RHandle << &m_BHandle;

    // set base config of the widget
    setWindowFlags(Qt::BypassWindowManagerHint
                   | Qt::WindowStaysOnTopHint
                   | Qt::FramelessWindowHint
                   | Qt::Tool);

    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    initShortcuts();

    // init content
    QPixmap fullScreenshot(ScreenGrabber().grabEntireDesktop());
    m_screenshot = new Screenshot(fullScreenshot, this);
    QSize size = fullScreenshot.size();
    // we need to increase by 1 the size to reach to the end of the screen
    setGeometry(0 ,0 , size.width()+1, size.height()+1);

    // create buttons
    m_buttonHandler = new ButtonHandler(this);
    updateButtons();
    m_buttonHandler->hide();
    // init interface color
    m_colorPicker = new ColorPicker(this);
    m_colorPicker->hide();
}

CaptureWidget::~CaptureWidget() {

}

// redefineButtons retrieves the buttons configured to be shown with the
// selection in the capture
void CaptureWidget::updateButtons() {
    ConfigHandler config;
    m_uiColor = config.getUIMainColor();
    m_contrastUiColor = config.getUIContrastColor();

    auto buttons = config.getButtons();
    QVector<CaptureButton*> vectorButtons;

    for (auto t: buttons) {
        CaptureButton *b = new CaptureButton(t, this);
        if (t == CaptureButton::TYPE_SELECTIONINDICATOR) {
            m_sizeIndButton = b;
        }
        b->setColor(m_uiColor);

        connect(b, &CaptureButton::hovered, this, &CaptureWidget::enterButton);
        connect(b, &CaptureButton::mouseExited, this, &CaptureWidget::leaveButton);
        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->getTool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
}

void CaptureWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_grabbing) { // grabWindow() should just get the background
        return;
    }
    // if we are creating a new modification to the screenshot we just draw
    // a temporal modification without antialiasing in the pencil tool for
    // performance. When we are not drawing we just shot the modified screenshot
    if (m_mouseIsClicked && m_state != CaptureButton::TYPE_MOVESELECTION) {
        painter.drawPixmap(0, 0, m_screenshot->paintTemporalModification(
                               m_modifications.last()));
    } else {
        painter.drawPixmap(0, 0, m_screenshot->getScreenshot());
    }

    QColor overlayColor(0, 0, 0, 160);
    painter.setBrush(overlayColor);
    QRect r = m_selection.normalized().adjusted(0, 0, -1, -1);
    QRegion grey(rect());
    grey = grey.subtracted(r);

    painter.setClipRegion(grey);
    painter.drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter.setClipRect(rect());

    if(m_showInitialMsg) {

        QRect helpRect = QGuiApplication::primaryScreen()->geometry();

        QString helpTxt = tr("Select an area with the mouse, or press Esc to exit."
                             "\nPress Enter to capture the screen."
                             "\nPress Right Click to show the color picker.");

        // We draw the white contrasting background for the text, using the
        //same text and options to get the boundingRect that the text will have.
        QColor rectColor = m_uiColor;
        rectColor.setAlpha(180);
        QColor textColor((CaptureButton::iconIsWhiteByColor(rectColor) ?
                              Qt::white : Qt::black));
        painter.setPen(QPen(textColor));
        painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
        QRectF bRect = painter.boundingRect(helpRect, Qt::AlignCenter, helpTxt);

        // These four calls provide padding for the rect
        bRect.setWidth(bRect.width() + 12);
        bRect.setHeight(bRect.height() + 10);
        bRect.setX(bRect.x() - 12);
        bRect.setY(bRect.y() - 10);

        painter.drawRect(bRect);

        // Draw the text:
        painter.setPen(textColor);
        painter.drawText(helpRect, Qt::AlignCenter, helpTxt);
    }

    if (!m_selection.isNull()) {
        // paint selection rect
        painter.setPen(m_uiColor);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r);

        // paint handlers
        updateHandles();
        painter.setBrush(m_uiColor);
        for(auto r: handleMask().rects()) {
            painter.drawRoundRect(r, 100, 100);
        }
    }
}

void CaptureWidget::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_rightClick = true;
        setCursor(Qt::ArrowCursor);
        m_colorPicker->move(e->pos().x()-m_colorPicker->width()/2,
                            e->pos().y()-m_colorPicker->height()/2);
        m_colorPicker->show();
        return;
    }

    if (e->button() == Qt::LeftButton) {
        m_showInitialMsg = false;
        m_mouseIsClicked = true;
        if (m_state != CaptureButton::TYPE_MOVESELECTION) {
            m_modifications.append(
                        new CaptureModification(m_state, e->pos(),
                                                m_colorPicker->getDrawColor(),
                                                this)
                        );
            return;
        }
        m_dragStartPoint = e->pos();
        m_selectionBeforeDrag = m_selection;
        m_buttonHandler->hide();
        if (!m_selection.contains(e->pos()) && !m_mouseOverHandle) {
            m_newSelection = true;
            m_selection = QRect();
            setCursor(Qt::CrossCursor);
        } else if (m_selection.contains(e->pos())){
            setCursor(Qt::ClosedHandCursor);
        }
    }
    update();
}

void CaptureWidget::mouseMoveEvent(QMouseEvent *e) {
    if (m_mouseIsClicked && m_state == CaptureButton::TYPE_MOVESELECTION) {
        m_mousePos = e->pos();

        if (m_newSelection) {
            m_selection = QRect(m_dragStartPoint, limitPointToRect(
                                    m_mousePos, rect())).normalized();
        } else if (m_mouseOverHandle == 0) {
            // Moving the whole selection
            QRect r = rect().normalized(), s = m_selectionBeforeDrag.normalized();
            QPoint p = s.topLeft() + e->pos() - m_dragStartPoint;
            r.setBottomRight(r.bottomRight() - QPoint(s.width(), s.height()));

            if (!r.isNull() && r.isValid()) {
                m_selection.moveTo(limitPointToRect(p, r));
            }
        } else {
            // Dragging a handle
            QRect r = m_selectionBeforeDrag;
            QPoint offset = e->pos() - m_dragStartPoint;

            bool symmetryMod = qApp->keyboardModifiers() & Qt::ShiftModifier;

            if (m_mouseOverHandle == &m_TLHandle || m_mouseOverHandle == &m_THandle
                    || m_mouseOverHandle == &m_TRHandle) { // dragging one of the top handles
                r.setTop(r.top() + offset.y());

                if (symmetryMod) {
                    r.setBottom(r.bottom() - offset.y());
                }
            }
            if (m_mouseOverHandle == &m_TLHandle || m_mouseOverHandle == &m_LHandle
                    || m_mouseOverHandle == &m_BLHandle) { // dragging one of the left handles
                r.setLeft(r.left() + offset.x());

                if (symmetryMod) {
                    r.setRight(r.right() - offset.x());
                }
            }
            if (m_mouseOverHandle == &m_BLHandle || m_mouseOverHandle == &m_BHandle
                    || m_mouseOverHandle == &m_BRHandle) { // dragging one of the bottom handles
                r.setBottom(r.bottom() + offset.y());

                if (symmetryMod) {
                    r.setTop(r.top() - offset.y());
                }
            }
            if (m_mouseOverHandle == &m_TRHandle || m_mouseOverHandle == &m_RHandle
                    || m_mouseOverHandle == &m_BRHandle) { // dragging one of the right handles
                r.setRight(r.right() + offset.x());

                if (symmetryMod) {
                    r.setLeft(r.left() - offset.x());
                }
            }
            r = r.normalized();
            r.setTopLeft(limitPointToRect(r.topLeft(), rect()));
            r.setBottomRight(limitPointToRect(r.bottomRight(), rect()));
            m_selection = r;
        }
        update();

    } else if (m_mouseIsClicked) {
        m_modifications.last()->addPoint(e->pos());
    } else {
        if (m_selection.isNull()) {
            update();
            return;
        }
        bool found = false;
        for (QRect *r: m_Handles) {
            if (r->contains(e->pos())) {
                m_mouseOverHandle = r;
                found = true;
                break;
            }
        }
        if (!found) {
            m_mouseOverHandle = 0;

            if (m_rightClick) {
                setCursor(Qt::ArrowCursor);
            } else if (m_selection.contains(e->pos()) && !m_onButton &&
                    m_state == CaptureButton::TYPE_MOVESELECTION) {
                setCursor(Qt::OpenHandCursor);
            } else if (m_onButton) {
                setCursor(Qt::ArrowCursor);
            } else {
                setCursor(Qt::CrossCursor);
            }
        } else if (m_state == CaptureButton::TYPE_MOVESELECTION){
            // cursor on the handlers
            if (m_mouseOverHandle == &m_TLHandle || m_mouseOverHandle == &m_BRHandle) {
                setCursor(Qt::SizeFDiagCursor);
            } else if (m_mouseOverHandle == &m_TRHandle || m_mouseOverHandle == &m_BLHandle) {
                setCursor(Qt::SizeBDiagCursor);
            } else if (m_mouseOverHandle == &m_LHandle || m_mouseOverHandle == &m_RHandle) {
                setCursor(Qt::SizeHorCursor);
            } else if (m_mouseOverHandle == &m_THandle || m_mouseOverHandle == &m_BHandle) {
                setCursor(Qt::SizeVerCursor);
            }
        }
    }
    update();
}

void CaptureWidget::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_colorPicker->hide();
        m_rightClick = false;
        return;
    // when we end the drawing of a modification in the capture we have to
    // register the last point and add the whole modification to the screenshot
    } else if (m_mouseIsClicked && m_state != CaptureButton::TYPE_MOVESELECTION) {
        m_screenshot->paintModification(m_modifications.last());
    }

    if (!m_selection.isNull() && !m_buttonHandler->isVisible()) {
        updateSizeIndicator();
        m_buttonHandler->updatePosition(m_selection, rect());
        m_buttonHandler->show();
    }
    m_mouseIsClicked = false;
    m_newSelection = false;

    if (m_state == CaptureButton::TYPE_MOVESELECTION && m_mouseOverHandle == 0 &&
            m_selection.contains(e->pos())) {
        setCursor(Qt::OpenHandCursor);
    }
    update();
}

void CaptureWidget::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Return) {
        copyScreenshot();
    } else if (e->key() == Qt::Key_Escape) {
        close();
    } else if (m_selection.isNull()) {
        return;
    } else if (e->key() == Qt::Key_Up
               && m_selection.top() > rect().top()) {
        m_selection.moveTop(m_selection.top()-1);
    } else if (e->key() == Qt::Key_Down
               && m_selection.bottom() < rect().bottom()) {
        m_selection.moveBottom(m_selection.bottom()+1);
    } else if (e->key() == Qt::Key_Left
               && m_selection.left() > rect().left()) {
        m_selection.moveLeft(m_selection.left()-1);
    } else if (e->key() == Qt::Key_Right
               && m_selection.right() < rect().right()) {
        m_selection.moveRight(m_selection.right()+1);
    }
    m_buttonHandler->updatePosition(m_selection, rect());
    update();
}

QString CaptureWidget::saveScreenshot(bool toClipboard) {
    hide();
    QString path;
    if (m_selection.isNull()) {
        path = m_screenshot->graphicalSave(QRect(), this);
    } else { // save full screen when no selection
        path = m_screenshot->graphicalSave(getExtendedSelection(), this);
    }
    if (!path.isEmpty()) {
        QString saveMessage(tr("Capture saved in "));
        Q_EMIT newMessage(saveMessage + path);
    }
    if (toClipboard) {
        copyScreenshot();
    }
    close();
    return path;
}

QString CaptureWidget::saveScreenshot(QString path, bool toClipboard) {
    ConfigHandler().setSavePath(path);
    QString savePath;
    if (m_selection.isNull()) {
        savePath = m_screenshot->fileSave();
    } else { // save full screen when no selection
        savePath = m_screenshot->fileSave(getExtendedSelection());
    }
    if (toClipboard) {
        copyScreenshot();
    }
    QString saveMessage(tr("Capture saved in "));
    Q_EMIT newMessage(saveMessage + savePath);
    close();
    return savePath;
}

void CaptureWidget::copyScreenshot() {
    if (m_selection.isNull()) {
        QApplication::clipboard()->setPixmap(m_screenshot->getScreenshot());
    } else { // copy full screen when no selection
        QApplication::clipboard()->setPixmap(m_screenshot->getScreenshot()
                                             .copy(getExtendedSelection()));
    }
    close();
}

void CaptureWidget::openURL(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString data = QString::fromUtf8(reply->readAll());
        QString imageID = data.split("\"").at(5);
        QString url = QString("http://i.imgur.com/%1.png").arg(imageID);
        bool successful = QDesktopServices::openUrl(url);
        if (!successful) {
            QMessageBox *openErrBox =
                    new QMessageBox(QMessageBox::Warning,
                                    QObject::tr("Resource Error"),
                                    QObject::tr("Unable to open the URL."));
            openErrBox->setModal(false);
            openErrBox->setAttribute(Qt::WA_DeleteOnClose);
            openErrBox->setWindowIcon(QIcon(":img/flameshot.png"));
            openErrBox->show();
        }
    } else {
        QMessageBox *netErrBox =
                new QMessageBox(QMessageBox::Warning, "Network Error",
                                reply->errorString());
        netErrBox->setModal(false);
        netErrBox->setAttribute(Qt::WA_DeleteOnClose);
        netErrBox->setWindowIcon(QIcon(":img/flameshot.png"));
        netErrBox->show();
    }
    close();
}

void CaptureWidget::uploadScreenshot() {
    QNetworkAccessManager *am = new QNetworkAccessManager(this);
    connect(am, &QNetworkAccessManager::finished, this,
            &CaptureWidget::openURL);
    if (m_selection.isNull()) {
        m_screenshot->uploadToImgur(am);
    } else {
        m_screenshot->uploadToImgur(am, getExtendedSelection());
    }
    hide();
    Q_EMIT newMessage(tr("Uploading image..."));
}

bool CaptureWidget::undo() {
    bool itemRemoved = false;
    if (!m_modifications.isEmpty()) {
        m_modifications.last()->deleteLater();
        m_modifications.pop_back();
        m_screenshot->paintBaseModifications(m_modifications);
        update();
        itemRemoved = true;
    }
    return itemRemoved;
}

void CaptureWidget::leftResize() {
    if (!m_selection.isNull() && m_selection.right() > m_selection.left()) {
        m_selection.setRight(m_selection.right()-1);
        m_buttonHandler->updatePosition(m_selection, rect());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::rightResize() {
    if (!m_selection.isNull() && m_selection.right() < rect().right()) {
        m_selection.setRight(m_selection.right()+1);
        m_buttonHandler->updatePosition(m_selection, rect());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::upResize() {
    if (!m_selection.isNull() && m_selection.bottom() > m_selection.top()) {
        m_selection.setBottom(m_selection.bottom()-1);
        m_buttonHandler->updatePosition(m_selection, rect());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::downResize() {
    if (!m_selection.isNull() && m_selection.bottom() < rect().bottom()) {
        m_selection.setBottom(m_selection.bottom()+1);
        m_buttonHandler->updatePosition(m_selection, rect());
        updateSizeIndicator();
        update();
    }
}

void CaptureWidget::setState(CaptureButton *b) {
    CaptureButton::ButtonType t = b->getButtonType();
    if(b->getTool()->isSelectable()) {
        if(t != m_state) {
            m_state = t;
            if (m_lastPressedButton) {
                m_lastPressedButton->setColor(m_uiColor);
            }
            m_lastPressedButton = b;
            m_lastPressedButton->setColor(m_contrastUiColor);
        } else {
            handleButtonSignal(CaptureTool::REQ_MOVE_MODE);
        }
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
        setCursor(Qt::CrossCursor);
        break;
    case CaptureTool::REQ_SAVE_SCREENSHOT:
        m_enableSaveWindow ?
                    saveScreenshot() :
                    saveScreenshot(ConfigHandler().getSavePath());
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
        uploadScreenshot();
        break;
    case CaptureTool::REQ_MOVE_MODE:
        m_state = CaptureButton::TYPE_MOVESELECTION;
        if (m_lastPressedButton) {
            m_lastPressedButton->setColor(m_uiColor);
            m_lastPressedButton = nullptr;
        }
        break;
    default:
        break;
    }
    update();
}

void CaptureWidget::leaveButton() {
    m_onButton = false;
}
void CaptureWidget::enterButton() {
    m_onButton = true;
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
}

void CaptureWidget::updateHandles() {
    QRect r = m_selection.normalized().adjusted(0, 0, -1, -1);
    int s2 = HANDLE_SIZE / 2;

    m_TLHandle.moveTopLeft(QPoint(r.x() - s2, r.y() - s2));
    m_TRHandle.moveTopRight(QPoint(r.right() + s2, r.y() - s2));
    m_BRHandle.moveBottomRight(QPoint(r.x() + r.width() + s2, r.bottom() + s2));
    m_BLHandle.moveBottomLeft(QPoint(QPoint(r.x() - s2, r.bottom() + s2)));

    m_LHandle.moveTopLeft(QPoint(r.x() - s2, r.y() + r.height() / 2 - s2));
    m_THandle.moveTopLeft(QPoint(r.x() + r.width() / 2 - s2, r.y() - s2));
    m_RHandle.moveTopRight(QPoint(r.right() + s2, r.y() + r.height() / 2 - s2));
    m_BHandle.moveBottomLeft(QPoint(r.x() + r.width() / 2 - s2, r.bottom() + s2));
}

void CaptureWidget::updateSizeIndicator() {
    // The grabbed region is everything which is covered by the drawn
    // rectangles (border included, that's the reason of the +2).
    if (m_sizeIndButton){
        m_sizeIndButton->setText(QString("%1\n%2")
                                     .arg(m_selection.width()+2)
                                     .arg(m_selection.height()+2));
    }
}

QRegion CaptureWidget::handleMask() const {
    // note: not normalized QRects are bad here, since they will not be drawn
    QRegion mask;
    foreach(QRect * rect, m_Handles) mask += QRegion(*rect);
    return mask;
}


QPoint CaptureWidget::limitPointToRect(const QPoint &p, const QRect &r) const {
    int x = r.x();
    int y = r.y();
    if (x < p.x()) {
        x = (p.x() < r.right()) ? p.x() : r.right();
    }
    if (y < p.y()) {
       y = (p.y() < r.bottom()) ? p.y() : r.bottom();
    }
    return QPoint(x,y);
}

QRect CaptureWidget::getExtendedSelection() const {
    auto devicePixelRatio = m_screenshot->getScreenshot().devicePixelRatio();

    return QRect(m_selection.left()   * devicePixelRatio,
                 m_selection.top()    * devicePixelRatio,
                 m_selection.width()  * devicePixelRatio,
                 m_selection.height() * devicePixelRatio);
}
