#ifndef CLIPTEST_H
#define CLIPTEST_H

#include <QtTest>
#include <QObject>

class ClipTest : public QObject
{
    Q_OBJECT
  public:
     ClipTest();

  signals:

  private slots:
     void testCaseConstructor();
     /**
      * @brief Test caching is not used with an empty, unused clip
      */
     void testCaseUsesCacher();
};

#endif // CLIPTEST_H
