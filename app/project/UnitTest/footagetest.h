#ifndef FOOTAGETEST_H
#define FOOTAGETEST_H

#include <QtTest>
#include <QObject>

class FootageTest : public QObject
{
    Q_OBJECT
  public:
    explicit FootageTest(QObject *parent = nullptr);

  signals:

  public slots:
  private slots:
    void testCaseIsImage();
};

#endif // FOOTAGETEST_H
