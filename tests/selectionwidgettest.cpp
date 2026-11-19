// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "widgets/capture/selectionwidget.h"

#include <QTest>

class SelectionWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void clickCommitsPresentedParent();
    void movingPastThresholdStartsManualSelection();
};

void SelectionWidgetTest::clickCommitsPresentedParent()
{
    QWidget overlay;
    overlay.resize(800, 600);
    SelectionWidget selection(Qt::magenta, &overlay);
    overlay.show();
    selection.hide();
    const QRect parent(50, 50, 400, 300);
    selection.setClickSelectionCandidate(parent);
    connect(&selection, &SelectionWidget::visibilityChanged, &selection, [&]() {
        selection.setClickSelectionCandidate({});
    });

    QTest::mousePress(
      &overlay, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    // A queued hover refresh after the press cannot shrink the chosen region.
    selection.setClickSelectionCandidate(QRect(90, 90, 30, 30));
    QTest::mouseMove(&overlay, QPoint(102, 101));
    QTest::mouseRelease(
      &overlay, Qt::LeftButton, Qt::NoModifier, QPoint(102, 101));
    QVERIFY(selection.isVisible());
    QCOMPARE(selection.geometry(), parent);
}

void SelectionWidgetTest::movingPastThresholdStartsManualSelection()
{
    QWidget overlay;
    overlay.resize(800, 600);
    SelectionWidget selection(Qt::magenta, &overlay);
    overlay.show();
    selection.hide();
    selection.setClickSelectionCandidate(QRect(50, 50, 400, 300));

    QTest::mousePress(
      &overlay, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    // Send the held-button move explicitly on all Qt platform plugins.
    QMouseEvent move(QEvent::MouseMove,
                     QPointF(200, 180),
                     QPointF(overlay.mapToGlobal(QPoint(200, 180))),
                     Qt::NoButton,
                     Qt::LeftButton,
                     Qt::NoModifier);
    QApplication::sendEvent(&overlay, &move);
    QTest::mouseRelease(
      &overlay, Qt::LeftButton, Qt::NoModifier, QPoint(200, 180));
    QVERIFY(selection.isVisible());
    // The existing resize path excludes its border pixels. Check the dragged
    // area, allowing that inset, rather than accepting the detected parent.
    const QRect dragged(QPoint(100, 100), QPoint(200, 180));
    QVERIFY(dragged.contains(selection.geometry()));
    QVERIFY(selection.geometry().contains(dragged.adjusted(2, 2, -2, -2)));
}

QTEST_MAIN(SelectionWidgetTest)

#include "selectionwidgettest.moc"
