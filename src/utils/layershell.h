// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Zaid & Contributors

#pragma once

class QScreen;
class QWidget;

namespace LayerShell {

bool available();
bool makeOverlay(QWidget* widget, QScreen* screen);

} // namespace
