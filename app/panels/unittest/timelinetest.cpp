#include "timelinetest.h"

#include <QtTest>

#include "panels/timeline.h"
#include "ui/timelinewidget.h"

TimelineTest::TimelineTest(QObject* parent) : QObject(parent)
{

}


void TimelineTest::testCaseAdjustTrackHeights()
{
  Timeline t;
  constexpr auto TRACKS = 10;
  t.audio_track_heights.fill(0, TRACKS);
  t.video_track_heights.fill(0, TRACKS);

  constexpr auto INC_VAL = 100;
  QVector<int> ref(TRACKS, INC_VAL);

  t.changeTrackHeight(t.audio_track_heights, INC_VAL);
  QCOMPARE(ref, t.audio_track_heights);

  t.changeTrackHeight(t.video_track_heights, INC_VAL*3);
  ref.fill(INC_VAL*3, TRACKS);
  QCOMPARE(ref, t.video_track_heights);

  t.changeTrackHeight(t.audio_track_heights, INC_VAL);
  ref.fill(INC_VAL*2, TRACKS);
  QCOMPARE(ref, t.audio_track_heights);

  t.changeTrackHeight(t.audio_track_heights, -INC_VAL);
  ref.fill(INC_VAL, TRACKS);
  QCOMPARE(ref, t.audio_track_heights);

  t.changeTrackHeight(t.video_track_heights, -INC_VAL*100);
  ref.fill(TRACK_MIN_HEIGHT, TRACKS);
  QCOMPARE(ref, t.video_track_heights);

}
