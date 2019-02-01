#include <QString>
#include <QtTest>

class SequenceItemTest : public QObject
{
    Q_OBJECT

public:
    SequenceItemTest();

private Q_SLOTS:
    void testCase1();
};

SequenceItemTest::SequenceItemTest()
{
}

void SequenceItemTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(SequenceItemTest)

#include "tst_sequenceitemtest.moc"
