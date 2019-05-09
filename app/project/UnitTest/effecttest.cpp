#include "effecttest.h"
#include <QtTest/QTest>
#include "project/effect.h"

Q_DECLARE_METATYPE(Capability)

EffectTest::EffectTest(QObject *parent)
  : QObject(parent)
{
  clip_ = std::make_shared<Clip>(std::make_shared<Sequence>());
}


void EffectTest::testCaseSetupWidget()
{
  EffectMeta em;
  Effect eff(clip_, em);
  eff.setupUi();
  QVERIFY(eff.hasCapability(Capability::SHADER) == false);
  QVERIFY(eff.glsl_.frag_.size() == 0);
  QVERIFY(eff.glsl_.vert_.size() == 0);

}


void EffectTest::testCaseIsEnabled()
{
  Effect eff(clip_);
  QVERIFY(eff.is_enabled() == false);
  eff.setupUi();
  QVERIFY(eff.is_enabled() == true);
}


void EffectTest::testCaseSetEnabled()
{
  Effect eff(clip_);
  eff.setupUi();
  eff.set_enabled(false);
  QVERIFY(eff.is_enabled() == false);
  eff.set_enabled(true);
  QVERIFY(eff.is_enabled() == true);
}


void EffectTest::testCaseDefaultCapabilities_data()
{
  QTest::addColumn<Capability>("inputType");
  QTest::addColumn<bool>("outputType");
  QTest::newRow("shdr") << Capability::SHADER << false;
  QTest::newRow("coord") << Capability::COORDS << false;
  QTest::newRow("superimp") << Capability::SUPERIMPOSE << false;
  QTest::newRow("img") << Capability::IMAGE << false;
}

void EffectTest::testCaseDefaultCapabilities()
{
  Effect eff(clip_);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == false);

  QFETCH(Capability, inputType);
  QFETCH(bool, outputType);

  QCOMPARE(eff.hasCapability(inputType), outputType);
}


void EffectTest::testCaseSetCapability()
{
  Effect eff(clip_);
  eff.setCapability(Capability::IMAGE);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == true);
  // check set doesn't effect previously set value
  eff.setCapability(Capability::IMAGE);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == true);
}


void EffectTest::testCaseClearCapability()
{
  Effect eff(clip_);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == false);
  eff.setCapability(Capability::IMAGE);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == true);
  eff.clearCapability(Capability::IMAGE);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == false);
  eff.clearCapability(Capability::IMAGE);
  QVERIFY(eff.hasCapability(Capability::IMAGE) == false);
}
