#include "sequencetest.h"

SequenceTest::SequenceTest()
{

}


void SequenceTest::testCaseDefaults()
{
    QVector<std::shared_ptr<Media>> mediaList;
    QString sequenceName = "Default";
    Sequence sqn(mediaList, sequenceName);

    QVERIFY(sqn.getName() == sequenceName);
    QVERIFY(sqn.getAudioFrequency() == 48000);
    QVERIFY(sqn.getAudioLayout() == 3);
    QVERIFY(qFuzzyCompare(sqn.getFrameRate(), 29.97));
    QVERIFY(sqn.getHeight() == 1080);
    QVERIFY(sqn.getWidth() == 1920);
}
