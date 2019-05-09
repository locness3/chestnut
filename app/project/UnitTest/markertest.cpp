#include "markertest.h"
#include <QtTest>

#include "project/marker.h"

MarkerTest::MarkerTest(QObject *parent) : QObject(parent)
{

}


void MarkerTest::testCaseLoad()
{
  QString data = "<marker>";
  data += "<name>test</name>";
  data += "<frame>2</frame>";
  data += "<comment>test Comment</comment>";
  data += "<duration>3</duration>";
  data += "<color>4294967295</color>"; //white
  data += "</marker>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();

  Marker mrkr;
  QVERIFY(mrkr.load(reader) == true);
  QVERIFY(mrkr.name == "test");
  QVERIFY(mrkr.frame == 2);
  QVERIFY(mrkr.comment_ == "test Comment");
  QVERIFY(mrkr.duration_ == 3);
  QVERIFY(mrkr.color_.rgb() == 4294967295);
}


void MarkerTest::testCaseLoadMissingName()
{
  QString data = "<marker>";
  data += "<frame>2</frame>";
  data += "<comment>test Comment</comment>";
  data += "<duration>3</duration>";
  data += "<color>4294967295</color>"; //white
  data += "</marker>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();

  Marker mrkr;
  QVERIFY(mrkr.load(reader) == false);
}

void MarkerTest::testCaseLoadMissingFrame()
{
  QString data = "<marker>";
  data += "<name>test</name>";
  data += "<comment>test Comment</comment>";
  data += "<duration>3</duration>";
  data += "<color>4294967295</color>"; //white
  data += "</marker>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();

  Marker mrkr;
  QVERIFY(mrkr.load(reader) == false);
}


void MarkerTest::testCaseLoadMissingComment()
{
  QString data = "<marker>";
  data += "<name>test</name>";
  data += "<frame>2</frame>";
  data += "<duration>3</duration>";
  data += "<color>4294967295</color>"; //white
  data += "</marker>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();

  Marker mrkr;
  QVERIFY(mrkr.load(reader) == false);
}

void MarkerTest::testCaseLoadMissingDuration()
{
  QString data = "<marker>";
  data += "<name>test</name>";
  data += "<frame>2</frame>";
  data += "<comment>test Comment</comment>";
  data += "<color>4294967295</color>"; //white
  data += "</marker>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();

  Marker mrkr;
  QVERIFY(mrkr.load(reader) == false);
}

void MarkerTest::testCaseLoadMissingColor()
{
  QString data = "<marker>";
  data += "<name>test</name>";
  data += "<frame>2</frame>";
  data += "<comment>test Comment</comment>";
  data += "<duration>3</duration>";
  data += "</marker>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();

  Marker mrkr;
  QVERIFY(mrkr.load(reader) == false);
}

void MarkerTest::testCaseSave()
{
  QString data;
  QXmlStreamWriter writer(&data);

  Marker mrkr;
  QVERIFY(mrkr.save(writer) == true);
  QVERIFY(data.size() > 0);
  QVERIFY(data.startsWith("<marker>") == true);
  QVERIFY(data.endsWith("</marker>") == true);
}
