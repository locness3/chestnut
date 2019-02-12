#include "sequenceitemtest.h"

#include "project/sequenceitem.h"
Q_DECLARE_METATYPE(project::SequenceItemType)

SequenceItemTest::SequenceItemTest()
{
}

void SequenceItemTest::testCaseConstructor_data()
{
    QTest::addColumn<project::SequenceItemType>("inputType");
    QTest::addColumn<project::SequenceItemType>("outputType");
    QTest::newRow("Clip") << project::SequenceItemType::CLIP << project::SequenceItemType::CLIP;
}

void SequenceItemTest::testCaseDefaults()
{
    project::SequenceItem item;
    QVERIFY2(item.type() == project::SequenceItemType::NONE, "Default SequenceItem type not NONE");
    QVERIFY2(item.name().isEmpty(), "Default SequenceItem name is not empty");
}

void SequenceItemTest::testCaseConstructor()
{
    QFETCH(project::SequenceItemType, inputType);
    QFETCH(project::SequenceItemType, outputType);

    project::SequenceItem item(inputType);

    QCOMPARE(item.type(), outputType);
}

void SequenceItemTest::testCaseNameSetter()
{
    project::SequenceItem item;
    item.setName("test name");
    QVERIFY(item.name() == "test name");
}
