#ifndef EFFECTTEST_H
#define EFFECTTEST_H

#include <QObject>

#include "project/clip.h"

class EffectTest : public QObject
{
    Q_OBJECT
  public:
    explicit EffectTest(QObject *parent = nullptr);

  signals:

  private slots:
    void testCaseSetupWidget();
    void testCaseIsEnabled();
    void testCaseSetEnabled();
    void testCaseDefaultCapabilities_data();
    void testCaseDefaultCapabilities();
    void testCaseSetCapability();
    void testCaseClearCapability();

  private:
    std::shared_ptr<Clip> clip_;
};

#endif // EFFECTTEST_H
