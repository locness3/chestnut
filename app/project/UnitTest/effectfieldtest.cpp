#include "effectfieldtest.h"

#include <QtTest>

#include "project/effectfield.h"

EffectFieldTest::EffectFieldTest(QObject *parent) : QObject(parent)
{

}


void EffectFieldTest::testCasePrevKey()
{
  EffectField fld(nullptr);
  QVERIFY(fld.previousKey(1).type == KeyframeType::UNKNOWN);
  EffectKeyframe key;
  key.time = 0;
  key.type = KeyframeType::BEZIER;
  fld.keyframes.append(key);
  QVERIFY(fld.previousKey(1).type == KeyframeType::BEZIER);
  QVERIFY(fld.previousKey(0).type == KeyframeType::UNKNOWN);
}

void EffectFieldTest::testCaseNextKey()
{
  EffectField fld(nullptr);
  QVERIFY(fld.nextKey(1).type == KeyframeType::UNKNOWN);
  EffectKeyframe key;
  key.time = 2;
  key.type = KeyframeType::BEZIER;
  fld.keyframes.append(key);
  QVERIFY(fld.nextKey(1).type == KeyframeType::BEZIER);
  QVERIFY(fld.nextKey(2).type == KeyframeType::UNKNOWN);
}
