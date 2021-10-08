// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/capturetool.h"
#include "src/widgets/capture/capturetoolbutton.h"
#include <QObject>

class CaptureTool;

class ToolFactory : public QObject
{
    Q_OBJECT

public:
    explicit ToolFactory(QObject* parent = nullptr);

    ToolFactory(const ToolFactory&) = delete;
    ToolFactory& operator=(const ToolFactory&) = delete;

    CaptureTool* CreateTool(CaptureTool::Type t, QObject* parent = nullptr);
};
