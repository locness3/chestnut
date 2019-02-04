#ifndef SEQUENCETEST_H
#define SEQUENCETEST_H

#include <QtTest>

#include "project/sequence.h"

class SequenceTest : public QObject
{
public:
    SequenceTest();
public slots:
    void testCaseDefaults();
};

#endif // SEQUENCETEST_H
