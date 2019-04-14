#ifndef FOOTAGETEST_H
#define FOOTAGETEST_H

#include <QtTest>
#include <QObject>

class FootageTest : public QObject
{
    Q_OBJECT
  public:
    explicit FootageTest(QObject *parent = nullptr);

  private slots:
    void testCaseIsImage();
    /**
     * Check an uninit footage does not save
     */
    void testCaseUninitSave();
};

#endif // FOOTAGETEST_H
