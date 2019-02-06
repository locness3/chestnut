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
    void testCaseCopy();
    void testCaseSetWidths_data();
    void testCaseSetWidths();
    void testCaseSetHeights_data();
    void testCaseSetHeights();
};

#endif // SEQUENCETEST_H
