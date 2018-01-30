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

#include <QObject>
#include "src/capture/widgets/capturebutton.h"
#include "src/capture/tools/capturetool.h"

class CaptureTool;

class ToolFactory : public QObject
{
    Q_OBJECT

public:
    enum ToolType {

    };

    explicit ToolFactory(QObject *parent = nullptr);

    ToolFactory(const ToolFactory &) = delete;
    ToolFactory & operator=(const ToolFactory &) = delete;

    CaptureTool* CreateTool(
            CaptureButton::ButtonType t,
            QObject *parent = nullptr);

};
