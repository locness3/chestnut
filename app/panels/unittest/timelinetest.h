#ifndef TIMELINETEST_H
#define TIMELINETEST_H

#include <QObject>

class TimelineTest : public QObject
{
    Q_OBJECT
  public:
    explicit TimelineTest(QObject *parent = nullptr);

  private slots:

    /**
     * @brief Check track heights are adjusted correctly and within limits
     */
    void testCaseAdjustTrackHeights();
};

#endif // TIMELINETEST_H
