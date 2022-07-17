// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "toolfactory.h"
#include "accept/accepttool.h"
#include "arrow/arrowtool.h"
#include "boldarrow/boldarrowtool.h"
#include "circle/circletool.h"
#include "circlecount/circlecounttool.h"
#include "copy/copytool.h"
#include "exit/exittool.h"
#include "imgupload/imguploadertool.h"
#include "invert/inverttool.h"
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
#include "text/texttool.h"
#include "undo/undotool.h"

ToolFactory::ToolFactory(QObject* parent)
  : QObject(parent)
{}

CaptureTool* ToolFactory::CreateTool(CaptureTool::Type t, QObject* parent)
{
#define if_TYPE_return_TOOL(TYPE, TOOL)                                        \
    case CaptureTool::TYPE:                                                    \
        return new TOOL(parent)

    switch (t) {
        if_TYPE_return_TOOL(TYPE_PENCIL, PencilTool);
        if_TYPE_return_TOOL(TYPE_DRAWER, LineTool);
        if_TYPE_return_TOOL(TYPE_ARROW, ArrowTool);
        if_TYPE_return_TOOL(TYPE_BOLD_ARROW, BoldArrowTool);
        if_TYPE_return_TOOL(TYPE_SELECTION, SelectionTool);
        if_TYPE_return_TOOL(TYPE_RECTANGLE, RectangleTool);
        if_TYPE_return_TOOL(TYPE_CIRCLE, CircleTool);
        if_TYPE_return_TOOL(TYPE_MARKER, MarkerTool);
        if_TYPE_return_TOOL(TYPE_SELECTIONINDICATOR, SizeIndicatorTool);
        if_TYPE_return_TOOL(TYPE_MOVESELECTION, MoveTool);
        if_TYPE_return_TOOL(TYPE_UNDO, UndoTool);
        if_TYPE_return_TOOL(TYPE_COPY, CopyTool);
        if_TYPE_return_TOOL(TYPE_SAVE, SaveTool);
        if_TYPE_return_TOOL(TYPE_EXIT, ExitTool);
        if_TYPE_return_TOOL(TYPE_IMAGEUPLOADER, ImgUploaderTool);
#if !defined(Q_OS_MACOS)
        if_TYPE_return_TOOL(TYPE_OPEN_APP, AppLauncher);
#endif
        if_TYPE_return_TOOL(TYPE_PIXELATE, PixelateTool);
        if_TYPE_return_TOOL(TYPE_REDO, RedoTool);
        if_TYPE_return_TOOL(TYPE_PIN, PinTool);
        if_TYPE_return_TOOL(TYPE_TEXT, TextTool);
        if_TYPE_return_TOOL(TYPE_CIRCLECOUNT, CircleCountTool);
        if_TYPE_return_TOOL(TYPE_SIZEINCREASE, SizeIncreaseTool);
        if_TYPE_return_TOOL(TYPE_SIZEDECREASE, SizeDecreaseTool);
        if_TYPE_return_TOOL(TYPE_INVERT, InvertTool);
        if_TYPE_return_TOOL(TYPE_ACCEPT, AcceptTool);
        default:
            return nullptr;
    }
}
