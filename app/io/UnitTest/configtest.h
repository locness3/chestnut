#ifndef CONFIGTEST_H
#define CONFIGTEST_H

#include <QtTest>

class ConfigTest : public QObject
{
    Q_OBJECT
  public:
    ConfigTest();

  signals:

  private slots:
    void testCaseNoLoopByDefault();

};

#endif // CONFIGTEST_H
