#include "mediatest.h"

#include "project/media.h"

MediaTest::MediaTest()
{

}


void MediaTest::testCaseConstructor()
{
  Media mda;
  QVERIFY(mda.getId() == 1);
  QVERIFY(mda.object<Footage>() == nullptr);
  QVERIFY(mda.object<Sequence>() == nullptr);
  QVERIFY(mda.get_type() == MediaType::NONE);
  QVERIFY(mda.get_name().isEmpty());
  QVERIFY(mda.childCount() == 0);
  QVERIFY(mda.child(0) == nullptr);
  QVERIFY(mda.columnCount() == 3);
  QVERIFY(mda.data(0,0).isValid());
  QVERIFY(mda.row() == 0);
  QVERIFY(mda.parentItem() == nullptr);
}


void MediaTest::testCaseConstructorWithParent()
{
  auto parent = std::make_shared<Media>();
  Media child(parent);
  QVERIFY(child.parentItem() != nullptr);
  QVERIFY(child.parentItem() == parent);
}


void MediaTest::testCaseSetAsFootage()
{
  Media mda;
  auto ftg = std::make_shared<Footage>();
  mda.set_footage(ftg);
  QVERIFY(mda.get_type() == MediaType::FOOTAGE);
  QVERIFY(mda.object<Footage>() != nullptr);
}

void MediaTest::testCaseSetAsSequence()
{
  Media mda;
  auto sqn = std::make_shared<Sequence>();
  mda.set_sequence(sqn);
  QVERIFY(mda.get_type() == MediaType::SEQUENCE);
  QVERIFY(mda.object<Sequence>() != nullptr);
}


void MediaTest::testCaseSetAsFolder()
{
  Media mda;
  mda.set_folder();
  QVERIFY(mda.get_type() == MediaType::FOLDER);
  QVERIFY(!mda.get_name().isEmpty());
}

void MediaTest::testCaseClear()
{
  Media mda;
  auto ftg = std::make_shared<Footage>();
  mda.set_footage(ftg);
  mda.clear_object();
  QVERIFY(mda.get_type() == MediaType::NONE);
  QVERIFY(mda.object<Footage>() == nullptr);
}
