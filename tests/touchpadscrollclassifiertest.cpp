// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "widgets/capture/touchpadscrollclassifier.h"

#include <QTest>

using Reason = TouchpadScrollClassifier::Reason;

class TouchpadScrollClassifierTest : public QObject
{
    Q_OBJECT

private slots:
    void mouseNotchWithoutContact();
    void wheelDuringContact();
    void sourceReportedByWindows();
    void inertiaAfterFingersLift();
    void newGestureAfterPause();
    void deltaFallbackWithoutRawInput();
    void highResolutionMouseWithRawInput();
    void timeWrapsAround();
};

void TouchpadScrollClassifierTest::mouseNotchWithoutContact()
{
    TouchpadScrollClassifier classifier;
    classifier.setTouchpadPresent(true);
    classifier.setRawInputWorks(true);
    QCOMPARE(classifier.classifyWheel(1000, 120, false), Reason::Mouse);
    QCOMPARE(classifier.classifyWheel(1100, -120, false), Reason::Mouse);
}

void TouchpadScrollClassifierTest::wheelDuringContact()
{
    TouchpadScrollClassifier classifier;
    classifier.contactReport(1000);
    QCOMPARE(classifier.classifyWheel(1010, -7, false), Reason::Contact);
    // A report can also arrive just after the wheel message it belongs to.
    classifier.contactReport(1300);
    QCOMPARE(classifier.classifyWheel(1290, -9, false), Reason::Contact);
}

void TouchpadScrollClassifierTest::sourceReportedByWindows()
{
    TouchpadScrollClassifier classifier;
    QCOMPARE(classifier.classifyWheel(1000, -120, true), Reason::Source);
}

void TouchpadScrollClassifierTest::inertiaAfterFingersLift()
{
    TouchpadScrollClassifier classifier;
    classifier.contactReport(1000);
    QCOMPARE(classifier.classifyWheel(1000, -40, false), Reason::Contact);
    // Inertia keeps scrolling for a while with no contact reports.
    quint32 time = 1250;
    for (int i = 0; i < 20; ++i) {
        time += 50;
        QCOMPARE(classifier.classifyWheel(time, -120, false), Reason::Sequence);
    }
}

void TouchpadScrollClassifierTest::newGestureAfterPause()
{
    TouchpadScrollClassifier classifier;
    classifier.setRawInputWorks(true);
    classifier.contactReport(1000);
    QCOMPARE(classifier.classifyWheel(1000, -40, false), Reason::Contact);
    // Much later, a mouse notch without contact is a mouse again.
    QCOMPARE(classifier.classifyWheel(5000, 120, false), Reason::Mouse);
    QCOMPARE(classifier.classifyWheel(5100, 120, false), Reason::Mouse);
}

void TouchpadScrollClassifierTest::deltaFallbackWithoutRawInput()
{
    TouchpadScrollClassifier classifier;
    classifier.setTouchpadPresent(true);
    QCOMPARE(classifier.classifyWheel(1000, -15, false), Reason::Delta);
    QCOMPARE(classifier.classifyWheel(1050, -120, false), Reason::Sequence);
    QCOMPARE(classifier.classifyWheel(9000, 120, false), Reason::Mouse);
    // Without a touchpad, fine deltas are a high-resolution mouse.
    TouchpadScrollClassifier noTouchpad;
    QCOMPARE(noTouchpad.classifyWheel(1000, -15, false), Reason::Mouse);
}

void TouchpadScrollClassifierTest::highResolutionMouseWithRawInput()
{
    TouchpadScrollClassifier classifier;
    classifier.setTouchpadPresent(true);
    classifier.setRawInputWorks(true);
    QCOMPARE(classifier.classifyWheel(1000, -15, false), Reason::Mouse);
}

void TouchpadScrollClassifierTest::timeWrapsAround()
{
    TouchpadScrollClassifier classifier;
    classifier.contactReport(0xFFFFFFF0U);
    QCOMPARE(classifier.classifyWheel(0x10, -7, false), Reason::Contact);
    QCOMPARE(classifier.classifyWheel(0x100, -120, false), Reason::Sequence);
}

QTEST_GUILESS_MAIN(TouchpadScrollClassifierTest)

#include "touchpadscrollclassifiertest.moc"
