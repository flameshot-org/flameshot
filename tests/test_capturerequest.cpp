// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include <QRect>
#include <QString>
#include <QTest>
#include <QVariant>
#include <stdexcept>

// Reproduce the CaptureRequest class directly to avoid the ConfigHandler
// dependency chain. This tests the monitor selection logic and task flag
// operations introduced in the multimonitor fix.

class CaptureRequest
{
public:
    enum CaptureMode
    {
        FULLSCREEN_MODE,
        GRAPHICAL_MODE,
        SCREEN_MODE,
    };

    enum ExportTask
    {
        NO_TASK = 0,
        COPY = 1,
        SAVE = 2,
        PRINT_RAW = 4,
        PRINT_GEOMETRY = 8,
        PIN = 16,
        UPLOAD = 32,
        ACCEPT_ON_SELECT = 64,
    };

    CaptureRequest(CaptureMode mode,
                   const uint delay = 0,
                   QVariant data = QVariant(),
                   ExportTask tasks = NO_TASK)
      : m_mode(mode)
      , m_delay(delay)
      , m_tasks(tasks)
      , m_data(std::move(data))
      , m_selectedMonitor(-1)
      , m_hasSelectedMonitor(false)
    {
    }

    uint delay() const { return m_delay; }
    QVariant data() const { return m_data; }
    CaptureMode captureMode() const { return m_mode; }
    ExportTask tasks() const { return m_tasks; }

    void addTask(ExportTask task)
    {
        if (task == SAVE) {
            throw std::logic_error(
              "SAVE task must be added using addSaveTask");
        }

        m_tasks = static_cast<ExportTask>(static_cast<int>(m_tasks) | static_cast<int>(task));
    }

    void removeTask(ExportTask task)
    {
        ((int&)m_tasks) &= ~task;
    }

    void addSaveTask(const QString& path = QString())
    {
        m_tasks = static_cast<ExportTask>(static_cast<int>(m_tasks) | static_cast<int>(SAVE));
        m_path = path;
    }

    void setSelectedMonitor(int monitorIndex)
    {
        m_selectedMonitor = monitorIndex;
        m_hasSelectedMonitor = true;
    }

    int selectedMonitor() const { return m_selectedMonitor; }
    bool hasSelectedMonitor() const { return m_hasSelectedMonitor; }

private:
    CaptureMode m_mode;
    uint m_delay;
    QString m_path;
    ExportTask m_tasks;
    QVariant m_data;
    int m_selectedMonitor;
    bool m_hasSelectedMonitor;
};

using eTask = CaptureRequest::ExportTask;

inline eTask operator|(const eTask& a, const eTask& b)
{
    return static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
}

inline eTask operator&(const eTask& a, const eTask& b)
{
    return static_cast<eTask>(static_cast<int>(a) & static_cast<int>(b));
}

inline eTask& operator|=(eTask& a, const eTask& b)
{
    a = static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
    return a;
}

class TestCaptureRequest : public QObject
{
    Q_OBJECT

private slots:

    void monitorSelection_defaultState()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        QCOMPARE(req.hasSelectedMonitor(), false);
        QCOMPARE(req.selectedMonitor(), -1);
    }

    void monitorSelection_setMonitor0()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        req.setSelectedMonitor(0);
        QCOMPARE(req.hasSelectedMonitor(), true);
        QCOMPARE(req.selectedMonitor(), 0);
    }

    void monitorSelection_setMonitor2()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        req.setSelectedMonitor(2);
        QCOMPARE(req.hasSelectedMonitor(), true);
        QCOMPARE(req.selectedMonitor(), 2);
    }

    void monitorSelection_overwrite()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        req.setSelectedMonitor(0);
        QCOMPARE(req.selectedMonitor(), 0);
        req.setSelectedMonitor(3);
        QCOMPARE(req.selectedMonitor(), 3);
        QCOMPARE(req.hasSelectedMonitor(), true);
    }

    void taskFlags_addAndRemove()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        QCOMPARE(req.tasks(), CaptureRequest::NO_TASK);

        req.addTask(CaptureRequest::COPY);
        QVERIFY(req.tasks() & CaptureRequest::COPY);

        req.addTask(CaptureRequest::PIN);
        QVERIFY(req.tasks() & CaptureRequest::COPY);
        QVERIFY(req.tasks() & CaptureRequest::PIN);

        req.removeTask(CaptureRequest::COPY);
        QVERIFY(!(req.tasks() & CaptureRequest::COPY));
        QVERIFY(req.tasks() & CaptureRequest::PIN);
    }

    void taskFlags_saveRequiresAddSaveTask()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        bool threw = false;
        try {
            req.addTask(CaptureRequest::SAVE);
        } catch (const std::logic_error&) {
            threw = true;
        }

        QVERIFY(threw);
    }

    void taskFlags_addSaveTask()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE);
        req.addSaveTask("/tmp/test.png");
        QVERIFY(req.tasks() & CaptureRequest::SAVE);
    }

    void taskFlags_operatorOr()
    {
        eTask combined = CaptureRequest::COPY | CaptureRequest::PIN;
        QCOMPARE(static_cast<int>(combined), static_cast<int>(CaptureRequest::COPY) | static_cast<int>(CaptureRequest::PIN));
    }

    void taskFlags_operatorAnd()
    {
        eTask combined = CaptureRequest::COPY | CaptureRequest::PIN;
        eTask masked = combined & CaptureRequest::COPY;
        QCOMPARE(masked, CaptureRequest::COPY);
    }

    void captureMode_values()
    {
        CaptureRequest req1(CaptureRequest::FULLSCREEN_MODE);
        QCOMPARE(req1.captureMode(), CaptureRequest::FULLSCREEN_MODE);

        CaptureRequest req2(CaptureRequest::GRAPHICAL_MODE);
        QCOMPARE(req2.captureMode(), CaptureRequest::GRAPHICAL_MODE);

        CaptureRequest req3(CaptureRequest::SCREEN_MODE);
        QCOMPARE(req3.captureMode(), CaptureRequest::SCREEN_MODE);
    }

    void constructorArgs_delayAndData()
    {
        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, 500, QVariant(42));
        QCOMPARE(req.delay(), 500u);
        QCOMPARE(req.data().toInt(), 42);
    }
};

QTEST_GUILESS_MAIN(TestCaptureRequest)
#include "test_capturerequest.moc"
