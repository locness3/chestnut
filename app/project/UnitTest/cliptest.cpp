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


void ClipTest::testCaseUsesCacherDefault()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  QVERIFY(clp.uses_cacher() == false);
}


void ClipTest::testCaseUsesCacher()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  auto mda =  std::make_shared<Media>();
  mda->setFootage(std::make_shared<Footage>());
  clp.timeline_info.media = mda;
  QVERIFY(clp.uses_cacher() == true);
  mda = std::make_shared<Media>();
  mda->setSequence(std::make_shared<Sequence>());
  clp.timeline_info.media = mda;
  QVERIFY(clp.uses_cacher() == false);
}
