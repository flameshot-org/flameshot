// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "inputtrace.h"

#include <QElapsedTimer>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>

namespace {
struct Trace
{
    Trace()
      : file(qEnvironmentVariable("FLAMESHOT_INPUT_LOG"))
    {
        elapsed.start();
        if (!file.fileName().isEmpty()) {
            file.open(QIODevice::WriteOnly | QIODevice::Append |
                      QIODevice::Text | QIODevice::Unbuffered);
        }
    }

    QFile file;
    QElapsedTimer elapsed;
    QMutex mutex;
};

Trace& trace()
{
    static Trace instance;
    return instance;
}
}

bool InputTrace::enabled()
{
    return trace().file.isOpen();
}

void InputTrace::write(const QString& line)
{
    Trace& t = trace();
    if (!t.file.isOpen()) {
        return;
    }
    const QMutexLocker lock(&t.mutex);
    t.file.write(
      QStringLiteral("%1 %2\n").arg(t.elapsed.elapsed(), 7).arg(line).toUtf8());
}
