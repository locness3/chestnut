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
  QVERIFY(clp.closingTransition() == nullptr);
  QVERIFY(clp.openingTransition() == nullptr);
  QVERIFY(clp.height() == -1);
  QVERIFY(clp.width() == -1);
  QVERIFY(clp.maximumLength() == -1);
  QVERIFY(clp.isActive(0) == false);
  QVERIFY(clp.isSelected(false) == false);
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
  mda->setFootage(std::make_shared<Footage>(mda));
  clp.timeline_info.media = mda;
  QVERIFY(clp.usesCacher() == true);
  mda = std::make_shared<Media>();
  mda->setSequence(std::make_shared<Sequence>());
  clp.timeline_info.media = mda;
  QVERIFY(clp.usesCacher() == false);
}


void ClipTest::testCaseMediaOpen()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  QVERIFY(clp.mediaOpen() == false);
  auto mda =  std::make_shared<Media>();
  mda->setFootage(std::make_shared<Footage>(mda));
  clp.timeline_info.media = mda;
  QVERIFY(clp.mediaOpen() == false);
  clp.finished_opening = true;
  QVERIFY(clp.mediaOpen() == true);
  mda->clearObject();
  QVERIFY(clp.mediaOpen() == false);
  mda->setSequence(std::make_shared<Sequence>());
  QVERIFY(clp.mediaOpen() == false);
}


void ClipTest::testCaseClipType()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  clp.timeline_info.track_ = 100;
  QVERIFY(clp.type() == ClipType::AUDIO);
  clp.timeline_info.track_ = -100;
  QVERIFY(clp.type() == ClipType::VISUAL);
  clp.timeline_info.track_ = 0;
  QVERIFY(clp.type() == ClipType::AUDIO);
}

void ClipTest::testCaseisSelected()
{
  auto seq = std::make_shared<Sequence>();
  Clip clp(seq);
  Selection sel;
  sel.track = 1;
  clp.timeline_info.track_ = 0;
  QVERIFY(clp.isSelected(sel) == false);
  clp.timeline_info.in = 0;
  clp.timeline_info.out = 1;
  clp.timeline_info.track_ = 1;
  QVERIFY(clp.isSelected(sel) == false);
  sel.in = 0;
  sel.out = 100;
  QVERIFY(clp.isSelected(sel) == true);
  sel.in = 1;
  QVERIFY(clp.isSelected(sel) == false);
  clp.timeline_info.in = 100;
  clp.timeline_info.out = 101;
  QVERIFY(clp.isSelected(sel) == false);
}

