// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

class QSharedMemory;

// Prevents multiple 'flameshot gui' instances from running at once. Returns
// nullptr if another instance already holds the lock.
QSharedMemory* guiMutexLock();

// Releases the lock acquired by guiMutexLock(), if held. Safe to call more
// than once, and safe to call even if guiMutexLock() was never called or
// already returned nullptr.
void releasePendingGuiMutex();
