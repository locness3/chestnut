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
    QVERIFY(sqn.audioFrequency() == 48000);
    QVERIFY(sqn.audioLayout() == 3);
    QVERIFY(qFuzzyCompare(sqn.frameRate(), 29.97));
    QVERIFY(sqn.height() == 1080);
    QVERIFY(sqn.width() == 1920);
    QVERIFY(sqn.clips_.size() == 0);
    QVERIFY(sqn.endFrame() == 0);
    int video_limit;
    int audio_limit;
    sqn.trackLimits(video_limit, audio_limit);
    QVERIFY(video_limit == 0);
    QVERIFY(audio_limit == 0);
}

void SequenceTest::testCaseCopy()
{
    Sequence sqnOrigin;
    auto sqnCopy = sqnOrigin.copy();
    QVERIFY(sqnOrigin.audioFrequency() == sqnCopy->audioFrequency());
    QVERIFY(sqnOrigin.audioLayout() == sqnCopy->audioLayout());
    QVERIFY(sqnOrigin.endFrame() == sqnCopy->endFrame());
    QVERIFY(qFuzzyCompare(sqnOrigin.frameRate(), sqnCopy->frameRate()));
    QVERIFY(sqnOrigin.height() == sqnCopy->height());
    QVERIFY(sqnOrigin.getName() != sqnCopy->getName());
    QVERIFY(sqnOrigin.width() == sqnCopy->width());
    QVERIFY(sqnOrigin.clips_.size() == sqnCopy->clips_.size());
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


void SequenceTest::testCaseSetFrameRate_data()
{
    QTest::addColumn<double>("rate");
    QTest::addColumn<bool>("result");
    QTest::newRow("negativeZero") << -0.0 << false;
    QTest::newRow("negative") << -1.0 << false;
    QTest::newRow("zero") << 0.0 << false;
    QTest::newRow("positive") << 60.0 << true;
    QTest::newRow("massivelyPositive") << 1000000.0 << true;

}

void SequenceTest::testCaseSetFrameRate()
{
    QFETCH(double, rate);
    QFETCH(bool, result);
    Sequence sqn;
    QCOMPARE(sqn.setFrameRate(rate), result);
}


void SequenceTest::testCaseSetFrequency_data()
{
    QTest::addColumn<int>("frequency");
    QTest::addColumn<bool>("result");
    QTest::newRow("negative") << -1 << false;
    QTest::newRow("zero") << 0 << true;
    QTest::newRow("typical") << 48000 << true;
    QTest::newRow("maximum") << 192000 << true;
    QTest::newRow("massive") << 1000000 << false;
}

void SequenceTest::testCaseSetFrequency()
{
    QFETCH(int, frequency);
    QFETCH(bool, result);
    Sequence sqn;
    QCOMPARE(sqn.setAudioFrequency(frequency), result);

}
