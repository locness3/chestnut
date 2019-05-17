#ifndef VIEWERTEST_H
#define VIEWERTEST_H

#include <QObject>

class ViewerTest : public QObject
{
    Q_OBJECT
  public:
    explicit ViewerTest(QObject *parent = nullptr);

  signals:

  private slots:
    /**
     * @brief Catch bug caught in issue #34
     */
    void testCaseCreateFootageSequenceAudioVideoClip();
};

#endif // VIEWERTEST_H
