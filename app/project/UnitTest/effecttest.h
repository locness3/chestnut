#ifndef EFFECTTEST_H
#define EFFECTTEST_H

#include <QObject>

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
};

#endif // EFFECTTEST_H
