#ifndef MEDIATEST_H
#define MEDIATEST_H

#include <QtTest>
#include <QObject>

class MediaTest : public QObject
{
    Q_OBJECT
public:
    MediaTest();

signals:

private slots:
    void testCaseConstructor();
    void testCaseConstructorWithParent();
    void testCaseSetAsFootage();
    void testCaseSetAsSequence();
    void testCaseSetAsFolder();
};

#endif // MEDIATEST_H
