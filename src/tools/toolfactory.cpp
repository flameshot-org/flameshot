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

#include "toolfactory.h"
#include "arrow/arrowtool.h"
#include "circle/circletool.h"
#include "circlecount/circlecounttool.h"
#include "copy/copytool.h"
#include "exit/exittool.h"
#include "imgur/imguruploadertool.h"
#include "launcher/applaunchertool.h"
#include "line/linetool.h"
#include "marker/markertool.h"
#include "move/movetool.h"
#include "pencil/penciltool.h"
#include "pin/pintool.h"
#include "pixelate/pixelatetool.h"
#include "rectangle/rectangletool.h"
#include "redo/redotool.h"
#include "save/savetool.h"
#include "selection/selectiontool.h"
#include "sizeindicator/sizeindicatortool.h"
#include "text/texttool.h"
#include "undo/undotool.h"
#include "eyedropper/eyedroppertool.h"

ToolFactory::ToolFactory(QObject* parent)
  : QObject(parent)
{}

CaptureTool*
ToolFactory::CreateTool(CaptureToolButton::ButtonType t, QObject* parent)
{
  CaptureTool* tool;
  switch (t) {
    case CaptureToolButton::TYPE_ARROW:
      tool = new ArrowTool(parent);
      break;
    case CaptureToolButton::TYPE_CIRCLE:
      tool = new CircleTool(parent);
      break;
    case CaptureToolButton::TYPE_COPY:
      tool = new CopyTool(parent);
      break;
    case CaptureToolButton::TYPE_EXIT:
      tool = new ExitTool(parent);
      break;
    case CaptureToolButton::TYPE_IMAGEUPLOADER:
      tool = new ImgurUploaderTool(parent);
      break;
    case CaptureToolButton::TYPE_DRAWER:
      tool = new LineTool(parent);
      break;
    case CaptureToolButton::TYPE_MARKER:
      tool = new MarkerTool(parent);
      break;
    case CaptureToolButton::TYPE_MOVESELECTION:
      tool = new MoveTool(parent);
      break;
    case CaptureToolButton::TYPE_PENCIL:
      tool = new PencilTool(parent);
      break;
    case CaptureToolButton::TYPE_RECTANGLE:
      tool = new RectangleTool(parent);
      break;
    case CaptureToolButton::TYPE_SAVE:
      tool = new SaveTool(parent);
      break;
    case CaptureToolButton::TYPE_SELECTION:
      tool = new SelectionTool(parent);
      break;
    case CaptureToolButton::TYPE_SELECTIONINDICATOR:
      tool = new SizeIndicatorTool(parent);
      break;
    case CaptureToolButton::TYPE_UNDO:
      tool = new UndoTool(parent);
      break;
    case CaptureToolButton::TYPE_REDO:
      tool = new RedoTool(parent);
      break;
    case CaptureToolButton::TYPE_OPEN_APP:
      tool = new AppLauncher(parent);
      break;
    case CaptureToolButton::TYPE_PIXELATE:
      tool = new PixelateTool(parent);
      break;
    case CaptureToolButton::TYPE_PIN:
      tool = new PinTool(parent);
      break;
    case CaptureToolButton::TYPE_TEXT:
      tool = new TextTool(parent);
      break;
    case CaptureToolButton::TYPE_CIRCLECOUNT:
      tool = new CircleCountTool(parent);
      break;
    case CaptureToolButton::TYPE_EYEDROPPER:
      tool = new EyedropperTool(parent);
      break;

    default:
      tool = nullptr;
      break;
  }
  return tool;
}
