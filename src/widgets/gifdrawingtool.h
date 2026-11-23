// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

/**
 * Tools available while a GIF is being recorded.
 *
 * NoTool means that the overlay is click-through: the user interacts with the
 * desktop normally and nothing is drawn.
 */
enum class GifDrawingTool
{
    NoTool = -1,
    Pencil = 0,
    Line,
    Arrow,
    Rectangle,
    Ellipse,
    Text,
    Number,
};
