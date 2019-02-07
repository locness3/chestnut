#include "cliptest.h"
#include "project/clip.h"

ClipTest::ClipTest()
{

}


void ClipTest::testCaseConstructor()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  QVERIFY(clp.sequence == seq);
  QVERIFY(clp.get_closing_transition() == nullptr);
  QVERIFY(clp.get_opening_transition() == nullptr);
  QVERIFY(clp.getHeight() == -1);
  QVERIFY(clp.getWidth() == -1);
  QVERIFY(clp.getMaximumLength() == -1);
  QVERIFY(clp.isActive(0) == false);
  QVERIFY(clp.is_selected(false) == false);
}
