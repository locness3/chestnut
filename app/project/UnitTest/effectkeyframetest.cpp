#include "effectkeyframetest.h"

#include <QXmlStreamWriter>
#include <QtTest>

#include "project/keyframe.h"

EffectKeyframeTest::EffectKeyframeTest(QObject *parent) : QObject(parent)
{

}

void EffectKeyframeTest::loadEmptyTestCase()
{
  QString data;
  QXmlStreamReader reader(data);
  EffectKeyframe kf;
  QVERIFY(kf.load(reader) == false);
}

void EffectKeyframeTest::loadMissingElementsTestCase()
{
  QString data = "<keyframe type='1'></keyframe>";
  QXmlStreamReader reader(data);
  reader.readNextStartElement();
  EffectKeyframe kf;
  QVERIFY(kf.load(reader) == false);
  QVERIFY(kf.type == static_cast<KeyframeType>(1));
}


void EffectKeyframeTest::loadPopulatedTestCase()
{
  QString data = "<keyframe type='1'>";
  data += "<value>1</value>";
  data += "<frame>2</frame>";
  data += "<prex>3</prex>";
  data += "<prey>4</prey>";
  data += "<postx>5</postx>";
  data += "<posty>6</posty>";
  data += "</keyframe>";

  QXmlStreamReader reader(data);
  reader.readNextStartElement();
  EffectKeyframe kf;
  QVERIFY(kf.load(reader) == true);
  QVERIFY(kf.type == static_cast<KeyframeType>(1));
  QVERIFY(kf.data == "1");
  QVERIFY(kf.time == 2);
  QVERIFY(qFuzzyCompare(kf.pre_handle_x,3.0));
  QVERIFY(qFuzzyCompare(kf.pre_handle_y,4.0));
  QVERIFY(qFuzzyCompare(kf.post_handle_x,5.0));
  QVERIFY(qFuzzyCompare(kf.post_handle_y,6.0));
}

void EffectKeyframeTest::saveTestCase()
{
  EffectKeyframe kf;

  kf.data.setValue(0.01);
  kf.time = 1000;
  kf.type = KeyframeType::BEZIER;

  QString buf;
  QXmlStreamWriter writer(&buf);
  QVERIFY(kf.save(writer) == true);
  QVERIFY(buf.size() > 0);
  QVERIFY(buf.startsWith("<keyframe") == true);
  QVERIFY(buf.endsWith("</keyframe>") == true);
}
