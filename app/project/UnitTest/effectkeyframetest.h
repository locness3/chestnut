#ifndef EFFECTKEYFRAMETEST_H
#define EFFECTKEYFRAMETEST_H

#include <QObject>

class EffectKeyframeTest : public QObject
{
    Q_OBJECT
  public:
    explicit EffectKeyframeTest(QObject *parent = nullptr);

  signals:

  private slots:

    void loadEmptyTestCase();
    void loadMissingElementsTestCase();
    void loadPopulatedTestCase();
    void saveTestCase();
};

#endif // EFFECTKEYFRAMETEST_H
