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
 
// Based on Lightscreen areadialog.h, Copyright 2017  Christian Kaiser <info@ckaiser.com.ar>
// released under the GNU GPL2  <https://www.gnu.org/licenses/gpl-2.0.txt>
 
// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007 Luca Gugelmann <lucag@student.ethz.ch>
// released under the GNU LGPL  <http://www.gnu.org/licenses/old-licenses/library.txt>

#pragma once

#include "capturebutton.h"
#include "src/capture/tools/capturetool.h"
#include "src/utils/confighandler.h"
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
class NotifierBox;

class CaptureWidget : public QWidget {
    Q_OBJECT

public:
    enum LaunchMode {
        WINDOW_MODE,
        FULLSCREEN,
    };

    explicit CaptureWidget(const uint id = 0,
                           const QString &forcedSavePath = QString(),
                           CaptureWidget::LaunchMode mode = LaunchMode::FULLSCREEN,
                           QWidget *parent = nullptr);
    ~CaptureWidget();


    void updateButtons();
    QPixmap pixmap();

signals:
    void captureTaken(uint id, QByteArray p);
    void captureFailed(uint id);

private slots:
    void copyScreenshot();
    void saveScreenshot();
    void uploadToImgur();
    void openWithProgram();
    bool undo();

    void leftResize();
    void rightResize();
    void upResize();
    void downResize();

    void setState(CaptureButton *);
    void handleButtonSignal(CaptureTool::Request r);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    void wheelEvent(QWheelEvent *);

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
    bool m_showInitialMsg;
    bool m_captureDone;
    bool m_toolIsForDrawing;

    const QString m_forcedSavePath;

    int m_thickness;
    int m_opacity;
    uint m_id;
    NotifierBox *m_notifierBox;

    // naming convention for handles
    // T top, B bottom, R Right, L left
    // 2 letters: a corner
    // 1 letter: the handle on the middle of the corresponding side
    QRect m_TLHandle, m_TRHandle, m_BLHandle, m_BRHandle;
    QRect m_LHandle, m_THandle, m_RHandle, m_BHandle;
    // Side Rects
    QRect m_LSide, m_TSide, m_RSide, m_BSide;
    // list containing the active habdlers
    QVector<QRect*> m_handles;
    QVector<QRect*> m_sides;

private:
    void initShortcuts();
    void updateHandles();
    void updateSizeIndicator();
    void updateCursor();

    // size of the handlers at the corners of the selection
    int handleSize();

    QRect extendedSelection() const;
    QVector<CaptureModification*> m_modifications;
    QPointer<CaptureButton> m_sizeIndButton;
    QPointer<CaptureButton> m_lastPressedButton;

    CaptureButton::ButtonType m_state;
    ButtonHandler *m_buttonHandler;

    QColor m_uiColor;
    QColor m_contrastUiColor;
    ColorPicker *m_colorPicker;

    ConfigHandler m_config;

};
