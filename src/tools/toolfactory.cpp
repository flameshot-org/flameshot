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

#include "toolfactory.h"
#include "arrow/arrowtool.h"
#include "circle/circletool.h"
#include "copy/copytool.h"
#include "exit/exittool.h"
#include "imgur/imguruploadertool.h"
#include "line/linetool.h"
#include "marker/markertool.h"
#include "move/movetool.h"
#include "pencil/penciltool.h"
#include "rectangle/rectangletool.h"
#include "save/savetool.h"
#include "selection/selectiontool.h"
#include "sizeindicator/sizeindicatortool.h"
#include "undo/undotool.h"
#include "launcher/applaunchertool.h"
#include "blur/blurtool.h"
#include "redo/redotool.h"
#include "pin/pintool.h"
#include "text/texttool.h"

ToolFactory::ToolFactory(QObject *parent) : QObject(parent) {

}

CaptureTool* ToolFactory::CreateTool(
        CaptureButton::ButtonType t,
        QObject *parent)
{
    CaptureTool *tool;
    switch (t) {
    case CaptureButton::TYPE_ARROW:
        tool = new ArrowTool(parent);
        break;
    case CaptureButton::TYPE_CIRCLE:
        tool = new CircleTool(parent);
        break;
    case CaptureButton::TYPE_COPY:
        tool = new CopyTool(parent);
        break;
    case CaptureButton::TYPE_EXIT:
        tool = new ExitTool(parent);
        break;
    case CaptureButton::TYPE_IMAGEUPLOADER:
        tool = new ImgurUploaderTool(parent);
        break;
    case CaptureButton::TYPE_DRAWER:
        tool = new LineTool(parent);
        break;
    case CaptureButton::TYPE_MARKER:
        tool = new MarkerTool(parent);
        break;
    case CaptureButton::TYPE_MOVESELECTION:
        tool = new MoveTool(parent);
        break;
    case CaptureButton::TYPE_PENCIL:
        tool = new PencilTool(parent);
        break;
    case CaptureButton::TYPE_RECTANGLE:
        tool = new RectangleTool(parent);
        break;
    case CaptureButton::TYPE_SAVE:
        tool = new SaveTool(parent);
        break;
    case CaptureButton::TYPE_SELECTION:
        tool = new SelectionTool(parent);
        break;
    case CaptureButton::TYPE_SELECTIONINDICATOR:
        tool = new SizeIndicatorTool(parent);
        break;
    case CaptureButton::TYPE_UNDO:
        tool = new UndoTool(parent);
        break;
    case CaptureButton::TYPE_REDO:
        tool = new RedoTool(parent);
        break;
    case CaptureButton::TYPE_OPEN_APP:
        tool = new AppLauncher(parent);
        break;
    case CaptureButton::TYPE_BLUR:
        tool = new BlurTool(parent);
        break;
    case CaptureButton::TYPE_PIN:
        tool = new PinTool(parent);
        break;
    case CaptureButton::TYPE_TEXT:
        tool = new TextTool(parent);
        break;
    default:
        tool = nullptr;
        break;
    }
    return tool;
}
