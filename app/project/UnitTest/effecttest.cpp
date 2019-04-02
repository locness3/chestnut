#include "effecttest.h"
#include <QtTest/QTest>
#include "project/effect.h"
#include "project/clip.h"

EffectTest::EffectTest(QObject *parent) : QObject(parent)
{

}


void EffectTest::testCaseSetupWidget()
{
  auto clp = std::make_shared<Clip>(std::make_shared<Sequence>());
  Effect eff(clp, nullptr);
  EffectMeta em;
  eff.setupControlWidget(em);
  QVERIFY(eff.enable_shader == false);
  QVERIFY(eff.fragPath.size() == 0);
  QVERIFY(eff.vertPath.size() == 0);

}


void EffectTest::testCaseIsEnabled()
{
  auto clp = std::make_shared<Clip>(std::make_shared<Sequence>());
  Effect eff(clp, nullptr);
  QVERIFY(eff.is_enabled() == true);
}


void EffectTest::testCaseSetEnabled()
{
  auto clp = std::make_shared<Clip>(std::make_shared<Sequence>());
  Effect eff(clp, nullptr);
  eff.set_enabled(false);
  QVERIFY(eff.is_enabled() == false);
  eff.set_enabled(true);
  QVERIFY(eff.is_enabled() == true);
}
