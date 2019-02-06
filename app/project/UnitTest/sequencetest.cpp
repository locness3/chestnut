#include "sequencetest.h"

SequenceTest::SequenceTest()
{

}


void SequenceTest::testCaseDefaults()
{
    QVector<std::shared_ptr<Media>> mediaList;
    QString sequenceName("Default");
    Sequence sqn(mediaList, sequenceName);

    QVERIFY(sqn.getName() == sequenceName);
    QVERIFY(sqn.getAudioFrequency() == 48000);
    QVERIFY(sqn.getAudioLayout() == 3);
    QVERIFY(qFuzzyCompare(sqn.getFrameRate(), 29.97));
    QVERIFY(sqn.getHeight() == 1080);
    QVERIFY(sqn.getWidth() == 1920);
    QVERIFY(sqn.clips.size() == 0);
    QVERIFY(sqn.getEndFrame() == 0);
    int video_limit;
    int audio_limit;
    sqn.getTrackLimits(video_limit, audio_limit);
    QVERIFY(video_limit == 0);
    QVERIFY(audio_limit == 0);
}

void SequenceTest::testCaseCopy()
{
    Sequence sqnOrigin;
    auto sqnCopy = sqnOrigin.copy();
    QVERIFY(sqnOrigin.getAudioFrequency() == sqnCopy->getAudioFrequency());
    QVERIFY(sqnOrigin.getAudioLayout() == sqnCopy->getAudioLayout());
    QVERIFY(sqnOrigin.getEndFrame() == sqnCopy->getEndFrame());
    QVERIFY(qFuzzyCompare(sqnOrigin.getFrameRate(), sqnCopy->getFrameRate()));
    QVERIFY(sqnOrigin.getHeight() == sqnCopy->getHeight());
    QVERIFY(sqnOrigin.getName() != sqnCopy->getName());
    QVERIFY(sqnOrigin.getWidth() == sqnCopy->getWidth());
    QVERIFY(sqnOrigin.clips.size() == sqnCopy->clips.size());
}

void SequenceTest::testCaseSetWidths_data()
{
    QTest::addColumn<int>("width");
    QTest::addColumn<bool>("result");
    QTest::newRow("negative") << -1 << false;
    QTest::newRow("oddNumber") << 121 << false;
    QTest::newRow("evenNumber") << 1920 << true;
    QTest::newRow("tooLarge") << 4098 << false;
    QTest::newRow("maximum") << 4096 << true;
}

void SequenceTest::testCaseSetWidths()
{
    QFETCH(int, width);
    QFETCH(bool, result);
    Sequence sqn;
    QCOMPARE(sqn.setWidth(width), result);
}


void SequenceTest::testCaseSetHeights_data()
{
    QTest::addColumn<int>("height");
    QTest::addColumn<bool>("result");
    QTest::newRow("negative") << -1 << false;
    QTest::newRow("oddNumber") << 121 << false;
    QTest::newRow("evenNumber") << 1920 << true;
    QTest::newRow("tooLarge") << 2162 << false;
    QTest::newRow("maximum") << 2160 << true;
}
void SequenceTest::testCaseSetHeights()
{

    QFETCH(int, height);
    QFETCH(bool, result);
    Sequence sqn;
    QCOMPARE(sqn.setHeight(height), result);
}
