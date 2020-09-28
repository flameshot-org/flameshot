// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

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
#include "sizedecrease/sizedecreasetool.h"
#include "sizeincrease/sizeincreasetool.h"
#include "sizeindicator/sizeindicatortool.h"
#include "src/utils/confighandler.h"
#include "text/texttool.h"
#include "undo/undotool.h"

ToolFactory::ToolFactory(QObject* parent)
  : QObject(parent)
{}

CaptureTool* ToolFactory::CreateTool(CaptureToolButton::ButtonType t,
                                     QObject* parent)
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
#if not defined(Q_OS_MACOS)
        case CaptureToolButton::TYPE_OPEN_APP:
            tool = new AppLauncher(parent);
            break;
#endif
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
        case CaptureToolButton::TYPE_SIZEINCREASE:
            tool = new SizeIncreaseTool(parent);
            break;
        case CaptureToolButton::TYPE_SIZEDECREASE:
            tool = new SizeDecreaseTool(parent);
            break;
        default:
            tool = nullptr;
            break;
    }
    return tool;
}
