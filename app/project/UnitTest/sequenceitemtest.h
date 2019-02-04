#ifndef SEQUENCEITEMTEST_H
#define SEQUENCEITEMTEST_H

#include <QtTest>

class SequenceItemTest : public QObject
{
    Q_OBJECT
public:
    SequenceItemTest();

private slots:
    void testCaseDefaults();
    void testCaseConstructor();
    void testCaseConstructor_data();
    void testCaseNameSetter();
};



#endif // SEQUENCEITEMTEST_H
