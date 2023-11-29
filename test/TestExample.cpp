#include <QtTest/QtTest>

class TestExample : public QObject {
    Q_OBJECT
private slots:
    void simpleTest() {
        QVERIFY(true);
    }
};

QTEST_MAIN(TestExample)
#include "TestExample.moc"

