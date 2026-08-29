// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "guimutex.h"

#include <QSharedMemory>
#include <QString>

static QSharedMemory* g_guiMutex = nullptr;

QSharedMemory* guiMutexLock()
{
    QString key = "org.flameshot.Flameshot-" APP_VERSION;
    auto* shm = new QSharedMemory(key);
#ifdef Q_OS_UNIX
    // Destroy shared memory if the last instance crashed on Unix
    shm->attach();
    delete shm;
    shm = new QSharedMemory(key);
#endif
    if (!shm->create(1)) {
        delete shm;
        return nullptr;
    }
    g_guiMutex = shm;
    return shm;
}

void releasePendingGuiMutex()
{
    if (g_guiMutex) {
        g_guiMutex->detach();
        delete g_guiMutex;
        g_guiMutex = nullptr;
    }
}
