#include "sequencetest.h"

SequenceTest::SequenceTest()
{

}


void SequenceTest::testCaseDefaults()
{
    Sequence sqn;

    QVERIFY(sqn.getName().isEmpty());
}
