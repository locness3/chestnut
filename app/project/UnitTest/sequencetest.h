#ifndef SEQUENCETEST_H
#define SEQUENCETEST_H

#include <QtTest>

#include "project/sequence.h"

class SequenceTest : public QObject
{
    Q_OBJECT
public:
    SequenceTest();
private slots:
    void testCaseDefaults();
};

#endif // SEQUENCETEST_H
