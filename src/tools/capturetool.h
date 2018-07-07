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

#pragma once

#include "src/tools/capturecontext.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include <QIcon>
#include <QPainter>

class CaptureTool : public QObject {
    Q_OBJECT

public:
    // Request actions on the main widget
    enum Request {
        // Call close() in the editor.
        REQ_CLOSE_GUI,
        // Call hide() in the editor.
        REQ_HIDE_GUI,
        // Select the whole screen.
        REQ_SELECT_ALL,
        // Disable the selection.
        REQ_HIDE_SELECTION,
        // Undo the last active modification in the stack.
        REQ_UNDO_MODIFICATION,
        // Redo the next modification in the stack.
        REQ_REDO_MODIFICATION,
        // Remove all the modifications.
        REQ_CLEAR_MODIFICATIONS,
        // Disable the active tool.
        REQ_MOVE_MODE,
        // Open the color picker under the mouse.
        REQ_SHOW_COLOR_PICKER,
        // Open/Close the side-panel.
        REQ_TOGGLE_SIDEBAR,
        // Call update() in the editor.
        REQ_REDRAW,
        // Append this tool to the undo/redo stack
        REQ_APPEND_TO_STACK,
        // Notify is the screenshot has been saved.
        REQ_CAPTURE_DONE_OK,
        // Instance this->widget()'s widget inside the editor under the mouse.
        REQ_ADD_CHILD_WIDGET,
        // Instance this->widget()'s widget as a window which closes after
        // closing the editor.
        REQ_ADD_CHILD_WINDOW,
        // Instance this->widget()'s widget which handles its own lifetime.
        REQ_ADD_EXTERNAL_WIDGETS,
    };

    explicit CaptureTool(QObject *parent = nullptr) : QObject(parent){}

    // Returns false when the tool is in an inconsistent state and shouldn't
    // be included in the tool undo/redo stack.
    virtual bool isValid() const = 0;
    // Close the capture after the process() call if the tool was activated
    // from a button press.
    virtual bool closeOnButtonPressed() const = 0;
    // If the tool keeps active after the selection.
    virtual bool isSelectable() const = 0;
    // Enable mouse preview.
    virtual bool showMousePreview() const = 0;

    // The icon of the tool.
    // inEditor is true when the icon is requested inside the editor
    // and false otherwise.
    virtual QIcon icon(const QColor &background,
                       bool inEditor) const = 0;
    // Name displayed for the tool, this could be translated with tr()
    virtual QString name() const = 0;
    // Codename for the tool, this hsouldn't change as it is used as ID
    // for the tool in the internals of Flameshot
    static QString nameID();
    // Short description of the tool.
    virtual QString description() const = 0;

    // if the type is TYPE_WIDGET the widget is loaded in the main widget.
    // If the type is TYPE_EXTERNAL_WIDGET it is created outside as an
    // individual widget.
    virtual QWidget* widget() {
        return nullptr;
    }
    // When the tool is selected this method is called and the widget is added
    // to the configuration panel inside the main widget.
    virtual QWidget* configurationWidget() {
        return nullptr;
    }
    // Permanent configuration used in the configuration outside of the
    // capture.
    virtual QWidget* permanentConfigurationWidget() {
        return nullptr;
    }
    // Return a copy of the tool
    virtual CaptureTool* copy(QObject *parent = nullptr) = 0;

    // revert changes
    virtual void undo(QPixmap &pixmap) = 0;
    // Called every time the tool has to draw
    // recordUndo indicates when the tool should save the information
    // for the undo(), if the value is false calling undo() after
    // that process should not modify revert the changes.
    virtual void process(QPainter &painter,
                         const QPixmap &pixmap,
                         bool recordUndo = false) = 0;
    // When the tool is selected, this is called when the mouse moves
    virtual void paintMousePreview(QPainter &painter, const CaptureContext &context) = 0;

signals:
    void requestAction(Request r);

protected:
    QString iconPath(const QColor &c) const {
        return ColorUtils::colorIsDark(c) ?
                    PathInfo::whiteIconPath() : PathInfo::blackIconPath();
    }

public slots:
    // On mouse release.
    virtual void drawEnd(const QPoint &p) = 0;
    // Mouse pressed and moving, called once a pixel.
    virtual void drawMove(const QPoint &p) = 0;
    // Called when the tool is activated.
    virtual void drawStart(const CaptureContext &context) = 0;
    // Called right after pressign the button which activates the tool.
    virtual void pressed(const CaptureContext &context) = 0;
    // Called when the color is changed in the editor.
    virtual void colorChanged(const QColor &c) = 0;
    // Called when the thickness of the tool is updated in the editor.
    virtual void thicknessChanged(const int th) = 0;
};
