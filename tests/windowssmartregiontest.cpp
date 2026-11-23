// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

// Runs the Windows backend against a real Notepad window, so CI checks that UI
// Automation finds elements inside a window and not only the window itself.

#include "widgets/capture/smartregiondetector.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QScopeGuard>
#include <QScreen>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QWindow>

class WindowsSmartRegionTest : public QObject
{
    Q_OBJECT

private slots:
    void findsElementsInsideWindow_data();
    void findsElementsInsideWindow();
};

namespace {
HWND findWindow(DWORD processId, const wchar_t* windowClass)
{
    struct Search
    {
        DWORD processId;
        HWND window;
    } search{ processId, nullptr };
    EnumWindows(
      [](HWND window, LPARAM value) -> BOOL {
          auto* search = reinterpret_cast<Search*>(value);
          DWORD owner = 0;
          GetWindowThreadProcessId(window, &owner);
          if (owner == search->processId && IsWindowVisible(window) &&
              GetAncestor(window, GA_ROOT) == window) {
              search->window = window;
              return FALSE;
          }
          return TRUE;
      },
      reinterpret_cast<LPARAM>(&search));
    // Some programs hand over to another process, e.g. Explorer to the shell.
    return search.window ? search.window : FindWindowW(windowClass, nullptr);
}
}

void WindowsSmartRegionTest::findsElementsInsideWindow_data()
{
    QTest::addColumn<QString>("program");
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<QString>("windowClass");
    QTest::addColumn<int>("minimumRegions");
    // The deepest region may cover at most this share of the window, so a
    // pane that fills the window does not count as an element.
    QTest::addColumn<double>("maximumLeafShare");
    QTest::addColumn<int>("timeoutMs");
    // Whether a full screen window covers the application during lookups,
    // like the capture overlay does.
    QTest::addColumn<bool>("covered");

    // A text area directly inside a window.
    QTest::newRow("notepad")
      << QStringLiteral("notepad.exe") << QStringList()
      << QStringLiteral("Notepad") << 2 << 1.0 << 5000 << true;
    // A file list with thousands of items, several levels deep.
    QTest::newRow("explorer")
      << QStringLiteral("explorer.exe")
      << QStringList{ QStringLiteral("C:\\Windows\\System32") }
      << QStringLiteral("CabinetWClass") << 3 << 0.25 << 5000 << true;
    // Chromium builds its accessibility tree only once a client asks for it,
    // and a web page has many elements: the case where lookups are slow.
    const QString chrome = QStringLiteral(
      "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe");
    QTest::newRow("chrome")
      << chrome << QStringList() << QStringLiteral("Chrome_WidgetWin_1") << 4
      << 0.25 << 10000 << false;
    // A covered Chromium window counts as hidden and may build its tree
    // several seconds later. It stays built while the application runs.
    QTest::newRow("chrome-covered")
      << chrome << QStringList() << QStringLiteral("Chrome_WidgetWin_1") << 4
      << 0.25 << 20000 << true;
}

void WindowsSmartRegionTest::findsElementsInsideWindow()
{
    QFETCH(QString, program);
    QFETCH(QStringList, arguments);
    QFETCH(QString, windowClass);
    QFETCH(int, minimumRegions);
    QFETCH(double, maximumLeafShare);
    QFETCH(int, timeoutMs);
    QFETCH(bool, covered);

    QTemporaryDir profile;
    if (windowClass == QLatin1String("Chrome_WidgetWin_1")) {
        if (!QFile::exists(program)) {
            QSKIP("Chrome is not installed");
        }
        // A long list of nested elements under the middle of the window.
        QFile page(profile.filePath(QStringLiteral("page.html")));
        QVERIFY(page.open(QIODevice::WriteOnly));
        page.write("<body style='margin:0'><main><section><ul>");
        for (int i = 0; i < 3000; ++i) {
            page.write(QStringLiteral("<li><span>Item %1</span> <a href='#'>"
                                      "link</a></li>")
                         .arg(i)
                         .toUtf8());
        }
        page.write("</ul></section></main></body>");
        page.close();
        arguments = { QStringLiteral("--no-first-run"),
                      QStringLiteral("--no-default-browser-check"),
                      QStringLiteral("--user-data-dir=") +
                        QDir::toNativeSeparators(
                          profile.filePath(QStringLiteral("profile"))),
                      QStringLiteral("--new-window"),
                      QUrl::fromLocalFile(page.fileName()).toString() };
    }

    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(10000)) {
        QSKIP("The program is not available");
    }
    HWND window = nullptr;
    const auto cleanup = qScopeGuard([&]() {
        if (window) {
            PostMessageW(window, WM_CLOSE, 0, 0);
        }
        process.kill();
    });
    QTRY_VERIFY_WITH_TIMEOUT(
      (window =
         findWindow(static_cast<DWORD>(process.processId()),
                    reinterpret_cast<const wchar_t*>(windowClass.utf16()))),
      15000);
    SetWindowPos(window, HWND_TOP, 100, 100, 900, 650, SWP_SHOWWINDOW);
    SetForegroundWindow(window);
    QTest::qWait(1500);

    RECT bounds{};
    GetWindowRect(window, &bounds);
    // The main content fills the middle of the window.
    const QPoint native((bounds.left + bounds.right) / 2,
                        (bounds.top + bounds.bottom) / 2);
    const qreal scale = QGuiApplication::primaryScreen()->devicePixelRatio();
    const QPoint logical(qRound(native.x() / scale),
                         qRound(native.y() / scale));

    auto detector = SmartRegionDetector::create();
    QVERIFY(detector);
    detector->snapshotWindows();
    // UI Automation starts on the worker thread.
    QTRY_VERIFY(detector->elementDetectionAvailable());

    QWindow overlay;
    if (covered) {
        overlay.setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::Tool);
        overlay.setGeometry(QGuiApplication::primaryScreen()->geometry());
        overlay.show();
        // Give applications time to notice that they are covered.
        QTest::qWait(2000);
    }

    // Ask repeatedly, like the capture widget does while the pointer rests:
    // the first lookups may only find the window.
    QList<QRect> regions;
    // Save what was found as a screenshot with the regions drawn on it, from
    // the deepest element (red) to the window, for people to check.
    const auto screenshot = qScopeGuard([&]() {
        overlay.hide();
        QTest::qWait(500);
        QScreen* screen = QGuiApplication::primaryScreen();
        QPixmap shot = screen->grabWindow(0);
        QPainter painter(&shot);
        painter.setRenderHint(QPainter::Antialiasing);
        const QList<QColor> colors{ Qt::red,    QColor(255, 140, 0),
                                    Qt::yellow, Qt::green,
                                    Qt::cyan,   Qt::blue,
                                    Qt::magenta };
        for (int i = int(regions.size()) - 1; i >= 0; --i) {
            const QRect region =
              regions.at(i).translated(-screen->geometry().topLeft());
            painter.setPen(
              QPen(colors.at(qMin(i, int(colors.size()) - 1)), i == 0 ? 4 : 2));
            painter.drawRect(region.adjusted(1, 1, -2, -2));
        }
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        const QString caption =
          QStringLiteral("%1: %2 regions under the cross, deepest in red")
            .arg(QString::fromLatin1(QTest::currentDataTag()))
            .arg(regions.size());
        painter.drawRect(10, 10, 560, 30);
        painter.drawText(QRect(10, 10, 560, 30), Qt::AlignCenter, caption);
        const QPoint cross = logical - screen->geometry().topLeft();
        painter.setPen(QPen(Qt::black, 3));
        painter.drawLine(cross - QPoint(12, 0), cross + QPoint(12, 0));
        painter.drawLine(cross - QPoint(0, 12), cross + QPoint(0, 12));
        painter.end();
        QDir().mkpath(QStringLiteral("screenshots"));
        shot.save(QStringLiteral("screenshots/%1.png")
                    .arg(QString::fromLatin1(QTest::currentDataTag())));
    });
    QElapsedTimer elapsed;
    elapsed.start();
    const auto area = [](const QRect& rect) {
        return double(rect.width()) * rect.height();
    };
    const auto foundElement = [&]() {
        regions = detector->regionsAt(logical);
        return regions.size() >= minimumRegions &&
               area(regions.first()) <= maximumLeafShare * area(regions.last());
    };
    QTRY_VERIFY2_WITH_TIMEOUT(
      foundElement(),
      qPrintable(QStringLiteral("Only found %1 regions, the deepest %2x%3")
                   .arg(regions.size())
                   .arg(regions.isEmpty() ? 0 : regions.first().width())
                   .arg(regions.isEmpty() ? 0 : regions.first().height())),
      timeoutMs);
    qInfo() << "Found" << regions.size() << "regions under" << logical
            << "after" << elapsed.elapsed() << "ms:" << regions;
    for (const QRect& region : regions) {
        QVERIFY(region.contains(logical));
    }
    // Ordered from the deepest element to the window.
    QVERIFY(regions.last().contains(regions.first()));
}

QTEST_MAIN(WindowsSmartRegionTest)

#include "windowssmartregiontest.moc"
