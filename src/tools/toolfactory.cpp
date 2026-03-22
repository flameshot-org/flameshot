// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "toolfactory.h"
#include "tools/accept/accepttool.h"
#include "tools/arrow/arrowtool.h"
#include "tools/circle/circletool.h"
#include "tools/circlecount/circlecounttool.h"
#include "tools/copy/copytool.h"
#include "tools/exit/exittool.h"
#ifdef ENABLE_IMGUR
#include "tools/imgupload/imguploadertool.h"
#endif
#include "tools/invert/inverttool.h"
#include "tools/launcher/applaunchertool.h"
#include "tools/line/linetool.h"
#include "tools/marker/markertool.h"
#include "tools/move/movetool.h"
#include "tools/pencil/penciltool.h"
#include "tools/pin/pintool.h"
#include "tools/pixelate/pixelatetool.h"
#include "tools/rectangle/rectangletool.h"
#include "tools/redo/redotool.h"
#include "tools/save/savetool.h"
#include "tools/selection/selectiontool.h"
#include "tools/sizedecrease/sizedecreasetool.h"
#include "tools/sizeincrease/sizeincreasetool.h"
#include "tools/text/texttool.h"
#include "tools/undo/undotool.h"

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
        if_TYPE_return_TOOL(TYPE_SELECTION, SelectionTool);
        if_TYPE_return_TOOL(TYPE_RECTANGLE, RectangleTool);
        if_TYPE_return_TOOL(TYPE_CIRCLE, CircleTool);
        if_TYPE_return_TOOL(TYPE_MARKER, MarkerTool);
        if_TYPE_return_TOOL(TYPE_MOVESELECTION, MoveTool);
        if_TYPE_return_TOOL(TYPE_UNDO, UndoTool);
        if_TYPE_return_TOOL(TYPE_COPY, CopyTool);
        if_TYPE_return_TOOL(TYPE_SAVE, SaveTool);
        if_TYPE_return_TOOL(TYPE_EXIT, ExitTool);
#ifdef ENABLE_IMGUR
        if_TYPE_return_TOOL(TYPE_IMAGEUPLOADER, ImgUploaderTool);
#endif
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
