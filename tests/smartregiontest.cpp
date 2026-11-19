// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "widgets/capture/smartregioncontroller.h"

#include <QSignalSpy>
#include <QTest>

namespace {
const QRect WINDOW(0, 0, 800, 600);
const QRect PANEL(50, 50, 400, 300);
const QRect BUTTON(70, 70, 100, 80);
const QRect OTHER_BUTTON(200, 70, 100, 80);

// Returns queued answers, repeating the last one, and counts lookups.
class FakeDetector : public SmartRegionDetector
{
public:
    explicit FakeDetector(QList<QList<QRect>> answers)
      : m_answers(std::move(answers))
    {}

    void snapshotWindows() override {}

    QList<QRect> regionsAt(const QPoint&) override
    {
        ++*m_lookups;
        return m_answers.size() > 1 ? m_answers.takeFirst()
                                    : m_answers.constFirst();
    }

    std::shared_ptr<int> lookups() const { return m_lookups; }

private:
    QList<QList<QRect>> m_answers;
    std::shared_ptr<int> m_lookups = std::make_shared<int>(0);
};

struct Fixture
{
    explicit Fixture(QList<QList<QRect>> answers)
    {
        auto detector = std::make_unique<FakeDetector>(std::move(answers));
        lookups = detector->lookups();
        controller =
          std::make_unique<SmartRegionController>(std::move(detector));
    }

    std::unique_ptr<SmartRegionController> controller;
    std::shared_ptr<int> lookups;
};
}

class SmartRegionTest : public QObject
{
    Q_OBJECT

private slots:
    void chainRunsFromDeepestElementToWindow();
    void chainOutsideWindowIsEmpty();
    void repeatedPositionDoesNotReplaceHighlight();
    void restingPointerRefinesLazyHierarchy();
    void refineNeverMakesHighlightCoarser();
    void suspendKeepsPressedRegion();
    void wheelWalksTowardParentAndBack();
    void chosenParentSurvivesMovementInside();
    void continuousScrollingIsRateLimited();
    void clearForgetsRegions();
};

void SmartRegionTest::chainRunsFromDeepestElementToWindow()
{
    const QRect window(10, 20, 400, 300);
    const QPoint pointer(80, 90);
    const QRect leaf(70, 80, 40, 30);
    const QRect parent(50, 60, 180, 120);
    const QList<QRect> elements{
        leaf,
        leaf,                    // duplicate
        QRect(78, 88, 2, 2),     // too small
        QRect(500, 500, 40, 40), // not under the pointer
        parent,
        QRect(0, 0, 600, 500), // clipped to the window
    };

    QCOMPARE(buildRegionChain(elements, window, pointer),
             QList<QRect>({ leaf, parent, window }));
}

void SmartRegionTest::chainOutsideWindowIsEmpty()
{
    QVERIFY(buildRegionChain({ BUTTON }, WINDOW, QPoint(900, 10)).isEmpty());
}

void SmartRegionTest::repeatedPositionDoesNotReplaceHighlight()
{
    // The first lookup sees a coarse tree, later ones the deepest element.
    Fixture f({ { PANEL, WINDOW }, { BUTTON, PANEL, WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    QCOMPARE(f.controller->current(), PANEL);

    // Qt sends a synthetic move at the same position right before a press.
    // It must not swap the highlighted region for one never shown.
    f.controller->pointerMoved(QPoint(100, 100));
    QCOMPARE(f.controller->current(), PANEL);
    QCOMPARE(*f.lookups, 1);
}

void SmartRegionTest::restingPointerRefinesLazyHierarchy()
{
    Fixture f({ { PANEL, WINDOW }, { BUTTON, PANEL, WINDOW } });
    QSignalSpy changed(f.controller.get(),
                       &SmartRegionController::currentChanged);
    f.controller->pointerMoved(QPoint(100, 100));
    QCOMPARE(f.controller->current(), PANEL);

    // The highlight settles on the deepest element without any input, so
    // what is shown before the click is what the click selects.
    QTRY_COMPARE(f.controller->current(), BUTTON);
    QCOMPARE(changed.size(), 2);
    QCOMPARE(changed.last().at(0).toRect(), PANEL);
    QCOMPARE(changed.last().at(1).toRect(), BUTTON);
}

void SmartRegionTest::refineNeverMakesHighlightCoarser()
{
    // A later lookup that timed out only returns the window.
    Fixture f({ { BUTTON, PANEL, WINDOW }, { WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    QTRY_VERIFY(*f.lookups >= 3);
    QCOMPARE(f.controller->current(), BUTTON);
}

void SmartRegionTest::suspendKeepsPressedRegion()
{
    Fixture f({ { PANEL, WINDOW }, { BUTTON, PANEL, WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    // The button is pressed on the coarse region before a refine runs.
    f.controller->suspend();
    QTest::qWait(300);
    QCOMPARE(f.controller->current(), PANEL);
    QCOMPARE(*f.lookups, 1);
}

void SmartRegionTest::wheelWalksTowardParentAndBack()
{
    Fixture f({ { BUTTON, PANEL, WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    f.controller->step(1, false);
    QCOMPARE(f.controller->current(), PANEL);
    f.controller->step(1, false);
    QCOMPARE(f.controller->current(), WINDOW);
    f.controller->step(1, false);
    QCOMPARE(f.controller->current(), WINDOW);
    f.controller->step(-1, false);
    f.controller->step(-1, false);
    f.controller->step(-1, false);
    QCOMPARE(f.controller->current(), BUTTON);
}

void SmartRegionTest::chosenParentSurvivesMovementInside()
{
    Fixture f({ { BUTTON, PANEL, WINDOW }, { OTHER_BUTTON, PANEL, WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    f.controller->step(1, false);
    QCOMPARE(f.controller->current(), PANEL);

    // Moving onto a sibling keeps the chosen parent.
    f.controller->pointerMoved(QPoint(220, 90));
    QTRY_COMPARE(f.controller->current(), PANEL);
}

void SmartRegionTest::continuousScrollingIsRateLimited()
{
    Fixture f({ { BUTTON, PANEL, WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    f.controller->step(5, true);
    f.controller->step(5, true);
    QCOMPARE(f.controller->current(), PANEL);
    QTest::qWait(200);
    f.controller->step(5, true);
    QCOMPARE(f.controller->current(), WINDOW);
}

void SmartRegionTest::clearForgetsRegions()
{
    Fixture f({ { BUTTON, PANEL, WINDOW } });
    f.controller->pointerMoved(QPoint(100, 100));
    f.controller->step(1, false);
    f.controller->clear();
    QVERIFY(f.controller->current().isNull());

    // After clearing, the same position is looked up again, and the earlier
    // wheel choice is forgotten.
    f.controller->pointerMoved(QPoint(100, 100));
    QCOMPARE(f.controller->current(), BUTTON);
}

QTEST_GUILESS_MAIN(SmartRegionTest)

#include "smartregiontest.moc"
