/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019 Jonathan Noble
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "timecodetest.h"

#include <QtTest>

#include "project/timecode.h"

using namespace chestnut::project;
using namespace media_handling;

TimeCodeTest::TimeCodeTest()
{

}

void TimeCodeTest::testCaseInit()
{
  Rational time_scale{1,1};
  Rational frame_rate{1,1};
  TimeCode tc(time_scale, frame_rate);
  QVERIFY(tc.timeScale() == time_scale);
  QVERIFY(tc.frameRate() == frame_rate);
  QVERIFY(tc.timestamp() == 0);
  QVERIFY(tc.toMillis() == 0);
  QVERIFY(tc.toFrames() == 0);
  QVERIFY(tc.toString() == "00:00:00:00");
}


void TimeCodeTest::testCaseInitSpecifiedTimestamp()
{
  Rational time_scale{1,1};
  Rational frame_rate{1,1};
  TimeCode tc(time_scale, frame_rate, 100);
  QVERIFY(tc.timestamp() == 100);
  QVERIFY(tc.toMillis() == 100'000);
  QVERIFY(tc.toFrames() == 100);
  QVERIFY(tc.toString() == "00:01:40:00");
}


void TimeCodeTest::testCaseInitPAL25()
{
  Rational time_scale{1,1000};
  Rational frame_rate{25,1};
  TimeCode tc(time_scale, frame_rate, 150); // 0.15s
  QVERIFY(tc.timestamp() == 150);
  QVERIFY(tc.toFrames() == 3);
  QVERIFY(tc.toMillis() == 150);
  QVERIFY(tc.toString() == "00:00:00:03");

  tc.setTimestamp(60'000); // 1min
  QVERIFY(tc.timestamp() == 60'000);
  QVERIFY(tc.toMillis() == 60'000);
  QVERIFY(tc.toFrames() == 1'500);
  QVERIFY(tc.toString() == "00:01:00:00");


  tc.setTimestamp(3'600'000); // 1hour
  QVERIFY(tc.timestamp() == 3'600'000);
  QVERIFY(tc.toMillis() == 3'600'000);
  QVERIFY(tc.toFrames() == 90'000);
  QVERIFY(tc.toString() == "01:00:00:00");


  tc.setTimestamp(100'000); // 1min
  QVERIFY(tc.timestamp() == 100'000);
  QVERIFY(tc.toMillis() == 100'000);
  QVERIFY(tc.toFrames() == 2'500);
  QVERIFY(tc.toString() == "00:01:40:00");
}
