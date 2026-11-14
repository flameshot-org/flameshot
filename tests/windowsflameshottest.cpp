// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

// Drives the real flameshot.exe on Windows the way a user does: starts a
// capture over Notepad, scrolls with the mouse wheel, simulates touchpad
// contact and presses Escape. The input log written by Flameshot tells what
// it did, and screenshots of each step are kept for people to check.

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPixmap>
#include <QProcess>
#include <QScopeGuard>
#include <QScreen>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

namespace {
struct WindowSearch
{
    DWORD processId;
    HWND window;
    qint64 area;
};

// The largest visible top-level window of a process.
HWND largestWindowOf(DWORD processId)
{
    WindowSearch search{ processId, nullptr, 0 };
    EnumWindows(
      [](HWND window, LPARAM value) -> BOOL {
          auto* search = reinterpret_cast<WindowSearch*>(value);
          DWORD owner = 0;
          GetWindowThreadProcessId(window, &owner);
          RECT rect{};
          if (owner == search->processId && IsWindowVisible(window) &&
              GetWindowRect(window, &rect)) {
              const qint64 area =
                qint64(rect.right - rect.left) * (rect.bottom - rect.top);
              if (area > search->area) {
                  search->area = area;
                  search->window = window;
              }
          }
          return TRUE;
      },
      reinterpret_cast<LPARAM>(&search));
    return search.window;
}

void sendKey(WORD key)
{
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;
    inputs[1] = inputs[0];
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void sendWheel(int delta)
{
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = static_cast<DWORD>(delta);
    SendInput(1, &input, sizeof(INPUT));
}

int countLines(const QString& path, const QString& text)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }
    return QString::fromUtf8(file.readAll()).count(text);
}
}

class WindowsFlameshotTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void escapeCancelsCapture();
    void wheelWalksParentThenChild();
    void touchpadScrollingFollowsFingers();
    void cleanupTestCase();

private:
    // Starts a capture over Notepad and waits for the overlay.
    void startCapture();
    void stopCapture();
    void screenshot(const QString& name);

    QTemporaryDir m_dir;
    QString m_log;
    QProcess m_notepad;
    QProcess m_flameshot;
    HWND m_overlay = nullptr;
    QPoint m_target;
};

void WindowsFlameshotTest::initTestCase()
{
    QVERIFY(QFile::exists(QStringLiteral(FLAMESHOT_EXE)));
    // Windows builds keep their config next to flameshot.exe by default,
    // and the installer build in the user profile; write both.
    QSettings portable(QFileInfo(QStringLiteral(FLAMESHOT_EXE)).absolutePath() +
                         QStringLiteral("/flameshot.ini"),
                       QSettings::IniFormat);
    QSettings profile(QSettings::IniFormat,
                      QSettings::UserScope,
                      QStringLiteral("flameshot"),
                      QStringLiteral("flameshot"));
    for (QSettings* config : { &portable, &profile }) {
        config->setValue(QStringLiteral("smartRegionDetection"), true);
        config->setValue(QStringLiteral("showHelp"), false);
        config->setValue(QStringLiteral("checkForUpdates"), false);
        config->setValue(QStringLiteral("showDesktopNotification"), false);
        config->setValue(QStringLiteral("showAbortNotification"), false);
        config->setValue(QStringLiteral("showStartupLaunchMessage"), false);
        config->sync();
    }
    m_log = m_dir.filePath(QStringLiteral("input.log"));
    QDir().mkpath(QStringLiteral("screenshots"));

    m_notepad.start(QStringLiteral("notepad.exe"));
    QVERIFY(m_notepad.waitForStarted(10000));
    HWND notepad = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT(
      (notepad = largestWindowOf(static_cast<DWORD>(m_notepad.processId()))) ||
        (notepad = FindWindowW(L"Notepad", nullptr)),
      15000);
    SetWindowPos(notepad, HWND_TOP, 100, 100, 900, 650, SWP_SHOWWINDOW);
    SetForegroundWindow(notepad);
    m_target = QPoint(550, 425);
    QTest::qWait(1000);
}

void WindowsFlameshotTest::startCapture()
{
    SetCursorPos(m_target.x(), m_target.y());
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("FLAMESHOT_INPUT_LOG"), m_log);
    environment.insert(QStringLiteral("FLAMESHOT_TEST_HOOKS"),
                       QStringLiteral("1"));
    m_flameshot.setProcessEnvironment(environment);
    m_flameshot.start(QStringLiteral(FLAMESHOT_EXE), { QStringLiteral("gui") });
    QVERIFY(m_flameshot.waitForStarted(10000));
    const auto processId = static_cast<DWORD>(m_flameshot.processId());
    // The overlay covers the screen.
    QTRY_VERIFY_WITH_TIMEOUT((m_overlay = largestWindowOf(processId)) &&
                               [this]() {
                                   RECT rect{};
                                   GetWindowRect(m_overlay, &rect);
                                   return rect.right - rect.left >= 800;
                               }(),
                             15000);
    // Let the highlight settle on the element under the pointer.
    QTest::qWait(1500);
}

void WindowsFlameshotTest::stopCapture()
{
    if (m_flameshot.state() != QProcess::NotRunning) {
        sendKey(VK_ESCAPE);
        if (!m_flameshot.waitForFinished(5000)) {
            m_flameshot.kill();
            m_flameshot.waitForFinished(5000);
        }
    }
    m_overlay = nullptr;
}

void WindowsFlameshotTest::screenshot(const QString& name)
{
    QGuiApplication::primaryScreen()->grabWindow(0).save(
      QStringLiteral("screenshots/%1.png").arg(name));
}

void WindowsFlameshotTest::escapeCancelsCapture()
{
    // Repeat, since Windows only sometimes refuses to bring the overlay to
    // the front.
    for (int attempt = 0; attempt < 3; ++attempt) {
        startCapture();
        const auto cleanup = qScopeGuard([this]() { stopCapture(); });
        QTRY_VERIFY2_WITH_TIMEOUT(GetForegroundWindow() == m_overlay,
                                  "The overlay is not in the foreground",
                                  2000);
        screenshot(QStringLiteral("escape-%1").arg(attempt));
        sendKey(VK_ESCAPE);
        QVERIFY2(m_flameshot.waitForFinished(5000),
                 "Escape did not close the overlay");
    }
}

void WindowsFlameshotTest::wheelWalksParentThenChild()
{
    startCapture();
    const auto cleanup = qScopeGuard([this]() { stopCapture(); });
    screenshot(QStringLiteral("wheel-0-start"));
    const QString parent = QStringLiteral("dir=parent");
    const QString child = QStringLiteral("dir=child");
    const int parents = countLines(m_log, parent);
    const int children = countLines(m_log, child);

    // A mouse wheel turned away from the user selects the parent.
    sendWheel(WHEEL_DELTA);
    QTRY_COMPARE_WITH_TIMEOUT(countLines(m_log, parent), parents + 1, 3000);
    screenshot(QStringLiteral("wheel-1-parent"));
    QTest::qWait(400);
    sendWheel(-WHEEL_DELTA);
    QTRY_COMPARE_WITH_TIMEOUT(countLines(m_log, child), children + 1, 3000);
    screenshot(QStringLiteral("wheel-2-child"));
}

void WindowsFlameshotTest::touchpadScrollingFollowsFingers()
{
    // With the default natural scrolling, fingers moving up send the same
    // wheel message as a wheel turned toward the user. Touchpad contact
    // cannot be injected, so the test hook stands in for it.
    startCapture();
    const auto cleanup = qScopeGuard([this]() { stopCapture(); });
    const UINT contact =
      RegisterWindowMessageW(L"FlameshotTestTouchpadContact");
    const QString parent = QStringLiteral("dir=parent");
    const QString child = QStringLiteral("dir=child");
    const int parents = countLines(m_log, parent);
    const int children = countLines(m_log, child);

    PostMessageW(m_overlay, contact, 0, 0);
    sendWheel(-WHEEL_DELTA); // fingers up
    QTRY_COMPARE_WITH_TIMEOUT(countLines(m_log, parent), parents + 1, 3000);
    QVERIFY(countLines(m_log, QStringLiteral("reason=contact")) > 0);
    screenshot(QStringLiteral("touchpad-1-parent"));
    QTest::qWait(1000);
    PostMessageW(m_overlay, contact, 0, 0);
    sendWheel(WHEEL_DELTA); // fingers down
    QTRY_COMPARE_WITH_TIMEOUT(countLines(m_log, child), children + 1, 3000);
    screenshot(QStringLiteral("touchpad-2-child"));
}

void WindowsFlameshotTest::cleanupTestCase()
{
    stopCapture();
    m_notepad.kill();
    // Keep the log next to the screenshots for people to read.
    QFile::copy(m_log, QStringLiteral("screenshots/input.log"));
}

QTEST_MAIN(WindowsFlameshotTest)

#include "windowsflameshottest.moc"
