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
    void testCaseSetFrameRate_data();
    void testCaseSetFrameRate();
    void testCaseSetFrequency_data();
    void testCaseSetFrequency();
    /**
     * @brief Check correct tracks are returned from a populated sequence
     */
    void testCaseTracks();
    /**
     * @brief Check correct clips are returned at a timeline position
     */
    void testCaseClips();

    void testCaseTrackCount();

    void testCaseTrackLocked();

    void testCaseTrackEnabled();

    void testCaseLockTrack();

    void testCaseEnableTrack();
};

#endif // SEQUENCETEST_H
