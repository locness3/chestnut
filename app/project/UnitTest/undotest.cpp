#include "undotest.h"
#include "project/undo.h"

UndoTest::UndoTest()
{

}


void UndoTest::testCaseNewSequenceCommandConstructor()
{
  auto seq = std::make_shared<Media>();
  auto parnt = std::make_shared<Media>();
  NewSequenceCommand cmd(seq, parnt, false);
  QVERIFY(cmd.seq_ == seq);
  QVERIFY(cmd.parent_ == parnt);
  QVERIFY(cmd.done_ == false);
  QVERIFY(cmd.old_project_changed_ == false);
}
