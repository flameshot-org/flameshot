// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Dmitrii Shcherbakov & Contributors

#pragma once

#include <QString>

namespace FileManagerUtils {
// Reveal the given file in the desktop's file manager, selecting it when
// the platform allows it. Falls back to opening the containing folder.
void revealFile(const QString& filePath);
}
