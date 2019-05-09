#ifndef MARKERTEST_H
#define MARKERTEST_H

#include <QObject>

class MarkerTest : public QObject
{
    Q_OBJECT
  public:
    explicit MarkerTest(QObject *parent = nullptr);

  signals:

  private slots:
    void testCaseLoad();
    void testCaseLoadMissingName();
    void testCaseLoadMissingFrame();
    void testCaseLoadMissingComment();
    void testCaseLoadMissingDuration();
    void testCaseLoadMissingColor();
    void testCaseSave();


};

#endif // MARKERTEST_H
