#ifndef UNDOTEST_H
#define UNDOTEST_H

#include <QObject>
#include <QtTest>

class UndoTest : public QObject
{
    Q_OBJECT
  public:
    UndoTest();

  private slots:
    void testCaseNewSequenceCommandConstructor();
};

#endif // UNDOTEST_H
