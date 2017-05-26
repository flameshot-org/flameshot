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
 
// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser <info@ckaiser.com.ar>
// released under the GNU GPL2  <https://www.gnu.org/licenses/gpl-2.0.txt>
 
// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007 Luca Gugelmann <lucag@student.ethz.ch>
// released under the GNU LGPL  <http://www.gnu.org/licenses/old-licenses/library.txt>

#ifndef CAPTUREWIDGET_H
#define CAPTUREWIDGET_H

#include "button.h"
#include "buttonhandler.h"
#include <QWidget>
#include <QPointer>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class CaptureModification;
class QNetworkAccessManager;
class QNetworkReply;
class ColorPicker;
class Screenshot;

class CaptureWidget : public QWidget {
    Q_OBJECT

    friend class Button;

public:
    explicit CaptureWidget(QWidget *parent = 0);
    ~CaptureWidget();

    void redefineButtons();

signals:
    void newMessage(QString);

private slots:
    void saveScreenshot();
    void copyScreenshot();
    void openURL(QNetworkReply *reply);
    void leaveButton();
    void enterButton();
    void undo();

    void leftResize();
    void rightResize();
    void upResize();
    void downResize();

    void setState(Button *);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

    QPoint limitPointToRect(const QPoint &p, const QRect &r) const;
    QRegion handleMask() const;

    // pixel map of the screen
    Screenshot* m_screenshot;

    QPoint m_dragStartPoint;
    QPoint m_mousePos;
    // pointer to the handlers under the mouse
    QRect *m_mouseOverHandle;
    QRect m_selection;
    QRect m_selectionBeforeDrag;
    // utility flags
    bool m_mouseIsClicked;
    bool m_rightClick;
    bool m_newSelection;
    bool m_grabbing;
    bool m_onButton;

    // naming convention for handles
    // T top, B bottom, R Right, L left
    // 2 letters: a corner
    // 1 letter: the handle on the middle of the corresponding side
    QRect m_TLHandle, m_TRHandle, m_BLHandle, m_BRHandle;
    QRect m_LHandle, m_THandle, m_RHandle, m_BHandle;
    // list containing the active habdlers
    QVector<QRect*> m_Handles;

private:
    void createCapture();
    void uploadScreenshot();
    void initShortcuts();
    void updateHandles();
    void updateSizeIndicator();

    QRect getExtendedSelection() const;
    QVector<CaptureModification> m_modifications;
    QPointer<Button> m_sizeIndButton;
    QPointer<Button> m_lastPressedButton;

    Button::Type m_state;
    ButtonHandler *m_buttonHandler;

    QColor m_uiColor;
    QColor m_contrastUiColor;
    ColorPicker *m_colorPicker;
};

#endif // CAPTUREWIDGET_H
