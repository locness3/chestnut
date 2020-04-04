/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2018  {{ organization }}
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
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTest>

#include "project/UnitTest/sequenceitemtest.h"
#include "project/UnitTest/sequencetest.h"
#include "project/UnitTest/mediatest.h"
#include "project/UnitTest/cliptest.h"
#include "project/UnitTest/footagetest.h"
#include "project/UnitTest/undotest.h"
#include "project/UnitTest/projectmodeltest.h"
#include "io/UnitTest/configtest.h"
#include "project/UnitTest/mediahandlertest.h"
#include "project/UnitTest/effecttest.h"
#include "project/UnitTest/effectkeyframetest.h"
#include "project/UnitTest/effectfieldtest.h"
#include "project/UnitTest/markertest.h"
#include "panels/unittest/histogramviewertest.h"
#include "panels/unittest/viewertest.h"
#include "panels/unittest/timelinetest.h"
#include "unittest/databasetest.h"

namespace
{
  int     argCount;
  char**  argVals;
}
template <typename T>
int runTest()
{
  T testItem;
  return QTest::qExec(&testItem, argCount, argVals);
}

int main(int argc, char** argv)
{
  QApplication a(argc, argv); // QWidgets need a constructed Qapplication
  argCount = argc;
  argVals = argv;
  ConfigTest testItem;
   // need 1 non-templated QTest::qExec for "Test Results" panel to work
  auto status = QTest::qExec(&testItem, argCount, argVals);
  status |= runTest<SequenceItemTest>();
  status |= runTest<chestnut::project::SequenceTest>();
  status |= runTest<chestnut::project::MediaTest>();
  status |= runTest<ClipTest>();
  status |= runTest<chestnut::project::FootageTest>();
  status |= runTest<UndoTest>();
  status |= runTest<ProjectModelTest>();
  status |= runTest<MediaHandlerTest>();
  status |= runTest<EffectTest>();
  status |= runTest<EffectFieldTest>();
  status |= runTest<EffectKeyframeTest>();
  status |= runTest<MarkerTest>();
  status |= runTest<panels::HistogramViewerTest>();
  status |= runTest<ViewerTest>();
  status |= runTest<TimelineTest>();
  status |= runTest<DatabaseTest>();
  return status;
}
