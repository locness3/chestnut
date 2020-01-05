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

namespace std
{
  char* toString(const string& value) {
    return QTest::toString(value.c_str());
  }
}

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


void TimeCodeTest::testCaseInitNTSC24()
{
  Rational time_scale{1,1000};
  Rational frame_rate{24000, 1001};
  TimeCode tc(time_scale, frame_rate, 600'000); // 10 minutes
  QCOMPARE(tc.timestamp(), 600'000);
  QCOMPARE(tc.toMillis(), 600'000);
  QCOMPARE(tc.toFrames(), 14'385);
  QCOMPARE(tc.toString(), "00:09:59:09");
}

void TimeCodeTest::testCaseInitNTSC30()
{
  Rational time_scale{1,1000};
  Rational frame_rate{30000,1001};
  TimeCode tc(time_scale, frame_rate);
  QCOMPARE(tc.timestamp(), 0);
  QCOMPARE(tc.toMillis(), 0);
  QCOMPARE(tc.toFrames(), 0);
  QCOMPARE(tc.toString(), "00:00:00;00");
  QCOMPARE(tc.toString(false), "00:00:00:00");

  tc.setTimestamp(59'966);
  QCOMPARE(tc.timestamp(), 59'966);
  QCOMPARE(tc.toMillis(), 59'966);
  QCOMPARE(tc.toFrames(), 1'797);
  QCOMPARE(tc.toString(), "00:00:59;27");
  QCOMPARE(tc.toString(false), "00:00:59:27");

  tc.setTimestamp(60'000);
  QCOMPARE(tc.timestamp(), 60'000);
  QCOMPARE(tc.toMillis(), 60'000);
  QCOMPARE(tc.toFrames(), 1'798);
  QCOMPARE(tc.toString(), "00:00:59;28");
  QCOMPARE(tc.toString(false), "00:00:59:28");

  tc.setTimestamp(60'033);
  QCOMPARE(tc.timestamp(), 60'033);
  QCOMPARE(tc.toMillis(), 60'033);
  QCOMPARE(tc.toFrames(), 1'799);
  QCOMPARE(tc.toString(), "00:00:59;29");
  QCOMPARE(tc.toString(false), "00:00:59:29");

  tc.setTimestamp(60'066);
  QCOMPARE(tc.timestamp(), 60'066);
  QCOMPARE(tc.toMillis(), 60'066);
  QCOMPARE(tc.toFrames(), 1'800);
  QCOMPARE(tc.toString(), "00:01:00;02");
  QCOMPARE(tc.toString(false), "00:01:00:00");

  tc.setTimestamp(60'099);
  QCOMPARE(tc.timestamp(), 60'099);
  QCOMPARE(tc.toMillis(), 60'099);
  QCOMPARE(tc.toFrames(), 1'801);
  QCOMPARE(tc.toString(), "00:01:00;03");
  QCOMPARE(tc.toString(false), "00:01:00:01");

  tc.setTimestamp(599'967); // 10minute - 1frame
  QCOMPARE(tc.timestamp(), 599'967);
  QCOMPARE(tc.toMillis(), 599'967);
  QCOMPARE(tc.toFrames(), 17'981);
  QCOMPARE(tc.toString(), "00:09:59;29");
  QCOMPARE(tc.toString(false), "00:09:59:11");

  tc.setTimestamp(600'000); // 10minute
  QCOMPARE(tc.timestamp(), 600'000);
  QCOMPARE(tc.toMillis(), 600'000);
  QCOMPARE(tc.toFrames(), 17'982);
  QCOMPARE(tc.toString(), "00:10:00;00");
  QCOMPARE(tc.toString(false), "00:09:59:12");

  tc.setTimestamp(600'033); // 10minute + 1frame
  QCOMPARE(tc.timestamp(), 600'033);
  QCOMPARE(tc.toMillis(), 600'033);
  QCOMPARE(tc.toFrames(), 17'983);
  QCOMPARE(tc.toString(), "00:10:00;01");
  QCOMPARE(tc.toString(false), "00:09:59:13");
}



void TimeCodeTest::testCaseNTSC30Frames()
{
  Rational time_scale{1,1};
  Rational frame_rate{30000, 1001};
  TimeCode tc(time_scale, frame_rate);
  QCOMPARE(tc.framesToSMPTE(0), "00:00:00;00");
  QCOMPARE(tc.framesToSMPTE(1), "00:00:00;01");
  QCOMPARE(tc.framesToSMPTE(2), "00:00:00;02");

  QCOMPARE(tc.framesToSMPTE(1797), "00:00:59;27");
  QCOMPARE(tc.framesToSMPTE(1798), "00:00:59;28");
  QCOMPARE(tc.framesToSMPTE(1799), "00:00:59;29");
  QCOMPARE(tc.framesToSMPTE(1800), "00:01:00;02");
  QCOMPARE(tc.framesToSMPTE(1801), "00:01:00;03");
  QCOMPARE(tc.framesToSMPTE(1802), "00:01:00;04");

  QCOMPARE(tc.framesToSMPTE(17979), "00:09:59;27");
  QCOMPARE(tc.framesToSMPTE(17980), "00:09:59;28");
  QCOMPARE(tc.framesToSMPTE(17981), "00:09:59;29");
  QCOMPARE(tc.framesToSMPTE(17982), "00:10:00;00");
  QCOMPARE(tc.framesToSMPTE(17983), "00:10:00;01");
  QCOMPARE(tc.framesToSMPTE(17984), "00:10:00;02");
}

void TimeCodeTest::testCaseInitNTSC60()
{
  Rational time_scale{1,1000};
  Rational frame_rate{60000,1001};
  TimeCode tc(time_scale, frame_rate);

  tc.setTimestamp(60'046);
  QCOMPARE(tc.timestamp(), 60'046);
  QCOMPARE(tc.toMillis(), 60'046);
  QCOMPARE(tc.toFrames(), 3'599);
  QCOMPARE(tc.toString(), "00:00:59;59");
  QCOMPARE(tc.toString(false), "00:00:59:59");

  tc.setTimestamp(60'066);
  QCOMPARE(tc.timestamp(), 60'066);
  QCOMPARE(tc.toMillis(), 60'066);
  QCOMPARE(tc.toFrames(), 3'600);
  QCOMPARE(tc.toString(), "00:01:00;04");
  QCOMPARE(tc.toString(false), "00:01:00:00");

  tc.setTimestamp(60'081);
  QCOMPARE(tc.timestamp(), 60'081);
  QCOMPARE(tc.toMillis(), 60'081);
  QCOMPARE(tc.toFrames(), 3'601);
  QCOMPARE(tc.toString(), "00:01:00;05");
  QCOMPARE(tc.toString(false), "00:01:00:01");

  tc.setTimestamp(599'983); // 10minute - 1frame
  QCOMPARE(tc.timestamp(), 599'983);
  QCOMPARE(tc.toMillis(), 599'983);
  QCOMPARE(tc.toFrames(), 35'963);
  QCOMPARE(tc.toString(), "00:09:59;59");
  QCOMPARE(tc.toString(false), "00:09:59:23");

  tc.setTimestamp(600'000); // 10minute
  QCOMPARE(tc.timestamp(), 600'000);
  QCOMPARE(tc.toMillis(), 600'000);
  QCOMPARE(tc.toFrames(), 35'964);
  QCOMPARE(tc.toString(), "00:10:00;00");
  QCOMPARE(tc.toString(false), "00:09:59:24");

  tc.setTimestamp(600'017); // 10minute + 1frame
  QCOMPARE(tc.timestamp(), 600'017);
  QCOMPARE(tc.toMillis(), 600'017);
  QCOMPARE(tc.toFrames(), 35'965);
  QCOMPARE(tc.toString(), "00:10:00;01");
  QCOMPARE(tc.toString(false), "00:09:59:25");
}


