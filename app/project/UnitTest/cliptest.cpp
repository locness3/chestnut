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
  QVERIFY(clp.height() == -1);
  QVERIFY(clp.width() == -1);
  QVERIFY(clp.maximumLength() == -1);
  QVERIFY(clp.isActive(0) == false);
  QVERIFY(clp.is_selected(false) == false);
}


void ClipTest::testCaseUsesCacherDefault()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  QVERIFY(clp.usesCacher() == false);
}


void ClipTest::testCaseUsesCacher()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  auto mda =  std::make_shared<Media>();
  mda->setFootage(std::make_shared<Footage>());
  clp.timeline_info.media = mda;
  QVERIFY(clp.usesCacher() == true);
  mda = std::make_shared<Media>();
  mda->setSequence(std::make_shared<Sequence>());
  clp.timeline_info.media = mda;
  QVERIFY(clp.usesCacher() == false);
}
