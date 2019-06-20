#ifndef EFFECTFIELDTEST_H
#define EFFECTFIELDTEST_H

#include <QObject>

class EffectFieldTest : public QObject
{
    Q_OBJECT
  public:
    explicit EffectFieldTest(QObject *parent = nullptr);

  signals:

  private slots:
    void testCasePrevKey();
    void testCaseNextKey();
    void testCaseSetValueColor();
};

#endif // EFFECTFIELDTEST_H
