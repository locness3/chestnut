#include "sequencetest.h"
#include "project/clip.h"

SequenceTest::SequenceTest()
{

}


void SequenceTest::testCaseDefaults()
{
  QVector<std::shared_ptr<Media>> mediaList;
  QString sequenceName("Default");
  Sequence sqn(mediaList, sequenceName);

  QVERIFY(sqn.name() == sequenceName);
  QVERIFY(sqn.audioFrequency() == 48000);
  QVERIFY(sqn.audioLayout() == 3);
  QVERIFY(qFuzzyCompare(sqn.frameRate(), 29.97));
  QVERIFY(sqn.height() == 1080);
  QVERIFY(sqn.width() == 1920);
  QVERIFY(sqn.clips_.size() == 0);
  QVERIFY(sqn.endFrame() == 0);
  int video_limit;
  int audio_limit;
  std::tie(video_limit,audio_limit) = sqn.trackLimits();
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
  QVERIFY(sqnOrigin.name() != sqnCopy->name());
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

void SequenceTest::testCaseTracks()
{
  auto sqn = std::make_shared<Sequence>();
  auto clip1 = std::make_shared<Clip>(sqn);
  clip1->timeline_info.track_ = 1;
  clip1->timeline_info.in = 0;
  clip1->timeline_info.out = 10;
  auto clip2 = std::make_shared<Clip>(sqn);
  clip2->timeline_info.track_ = 2;
  clip2->timeline_info.in = 0;
  clip2->timeline_info.out = 10;
  sqn->addClip(clip1);
  sqn->addClip(clip2);

  QVERIFY(sqn->tracks(true).empty() == true);
  QVERIFY(sqn->tracks(false).size() == 2);

}

void SequenceTest::testCaseClips()
{
  auto sqn = std::make_shared<Sequence>();
  auto clip1 = std::make_shared<Clip>(sqn);
  clip1->timeline_info.track_ = 1;
  clip1->timeline_info.in = 0;
  clip1->timeline_info.out = 10;
  auto clip2 = std::make_shared<Clip>(sqn);
  clip2->timeline_info.track_ = 2;
  clip2->timeline_info.in = 0;
  clip2->timeline_info.out = 20;
  sqn->clips_.append(clip1);
  sqn->clips_.append(clip2);

  // in range of both clips
  auto clips = sqn->clips(5);
  QVERIFY(clips.size() == 2);
  QVERIFY(clips.contains(clip1) == true);
  QVERIFY(clips.contains(clip2) == true);
  // out of range of both clips
  clips = sqn->clips(100);
  QVERIFY(clips.empty() == true);
  // in range of just clip2
  clips = sqn->clips(11);
  QVERIFY(clips.size() == 1);
  QVERIFY(clips.contains(clip2) == true);
}


void SequenceTest::testCaseTrackCount()
{
  auto sqn = std::make_shared<Sequence>();

  // by default 0 video tracks
  QVERIFY(sqn->trackCount(true) == 0);
  // by default 0 audio tracks
  QVERIFY(sqn->trackCount(true) == 0);

  //negative numbers are video tracks
  auto clip1 = std::make_shared<Clip>(sqn);
  clip1->timeline_info.track_ = 0;
  sqn->clips_.append(clip1);
  auto clip2 = std::make_shared<Clip>(sqn);
  clip2->timeline_info.track_ = 1;
  sqn->clips_.append(clip2);
  auto clip3 = std::make_shared<Clip>(sqn);
  clip3->timeline_info.track_ = -1;
  sqn->clips_.append(clip3);

  QVERIFY(sqn->trackCount(true) == 1);
  QVERIFY(sqn->trackCount(false) == 2);

}

void SequenceTest::testCaseTrackLocked()
{
  Sequence sqn;

  // By default, a track isn't locked
  QVERIFY(sqn.trackLocked(0) == false);

  sqn.lockTrack(1, true);
  QVERIFY(sqn.trackLocked(1) == true);
  sqn.lockTrack(1, false);
  QVERIFY(sqn.trackLocked(1) == false);
}

void SequenceTest::testCaseTrackEnabled()
{
  Sequence sqn;
  sqn.addTrack(1, true);
  sqn.enableTrack(1);
  QVERIFY(sqn.trackEnabled(1) == true);
  sqn.disableTrack(1);
  QVERIFY(sqn.trackEnabled(1) == false);
}


void SequenceTest::testCaseLockTrack()
{
  Sequence sqn;
  sqn.lockTrack(0, true);
  QVERIFY(sqn.trackLocked(0) == true);
  sqn.lockTrack(0, false);
  QVERIFY(sqn.trackLocked(0) == false);
}

void SequenceTest::testCaseEnableTrack()
{
  Sequence sqn;
  sqn.addTrack(0, true);
  sqn.enableTrack(0);
  QVERIFY(sqn.trackEnabled(0) == true);
  sqn.disableTrack(0);
  QVERIFY(sqn.trackEnabled(0) == false);
}

void SequenceTest::testCaseAddClip()
{
  Sequence sqn;
  auto clp = std::make_shared<Clip>(nullptr);
  QVERIFY(sqn.addClip(clp) == true);
}

void SequenceTest::testCaseAddClipDuplicate()
{
  Sequence sqn;
  auto clp = std::make_shared<Clip>(nullptr);
  QVERIFY(sqn.addClip(clp) == true);
  QVERIFY(sqn.addClip(clp) == false);
}


void SequenceTest::testCaseAddClipNull()
{
  Sequence sqn;
  ClipPtr clp = nullptr;
  QVERIFY(sqn.addClip(clp) == false);
}
