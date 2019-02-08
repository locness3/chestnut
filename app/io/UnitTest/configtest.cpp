#include "configtest.h"

#include "io/config.h"

ConfigTest::ConfigTest()
{

}


void ConfigTest::testCaseNoLoopByDefault()
{
  Config cfg;
  QVERIFY(cfg.loop == false);
}
