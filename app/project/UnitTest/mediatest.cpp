#include "mediatest.h"

#include "project/media.h"

MediaTest::MediaTest()
{

}


void MediaTest::testCaseConstructor()
{
    Media mda;
    QVERIFY(mda.getId() == 1);
    QVERIFY(mda.get_object<Footage>() == nullptr);
    QVERIFY(mda.get_object<Sequence>() == nullptr);
    QVERIFY(mda.get_type() == MediaType::NONE);
    QVERIFY(mda.get_name().isEmpty());
    QVERIFY(mda.childCount() == 0);
    QVERIFY(mda.child(0) == nullptr);
    QVERIFY(mda.columnCount() == 3);
    QVERIFY(mda.data(0,0).isNull());
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
