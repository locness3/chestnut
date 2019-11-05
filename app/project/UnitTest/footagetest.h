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
     * @brief Check an uninit footage does not save
     */
    void testCaseUninitSave();
    /**
     * @brief Test save when parent media is null
     */
    void testCaseNullMediaSave();
    /**
     * @brief Test sunny-day scenario save
     */
    void testCaseSave();
    /**
     * @brief Save a footage object and load another one with the result
     */
    void testCaseSaveAndLoad();

};

#endif // FOOTAGETEST_H
