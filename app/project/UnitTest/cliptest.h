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
};

#endif // CLIPTEST_H
