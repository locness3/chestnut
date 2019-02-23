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
     void testCaseUsesCacherDefault();
     /**
      * @brief Test caching used with a Media item assigned of type footage only
      */
     void testCaseUsesCacher();

     /**
      * @brief Test that the Clip's media is open
      */
     void testCaseMediaOpen();
};

#endif // CLIPTEST_H
