#include <QTest>

#include "project/UnitTest/sequenceitemtest.h"
#include "project/UnitTest/sequencetest.h"
#include "project/UnitTest/mediatest.h"
#include "project/UnitTest/cliptest.h"
#include "io/UnitTest/configtest.h"

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
  argCount = argc;
  argVals = argv;
  int status = runTest<ConfigTest>();
  status |= runTest<SequenceItemTest>();
  status |= runTest<SequenceTest>();
  status |= runTest<MediaTest>();
  status |= runTest<ClipTest>();
  return status;
}
