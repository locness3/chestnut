#ifndef HISTOGRAMVIEWERTEST_H
#define HISTOGRAMVIEWERTEST_H

#include <QObject>

namespace panels {
  class HistogramViewerTest : public QObject
  {
      Q_OBJECT
    public:
      HistogramViewerTest();

    private slots:
      void testCaseSetup();
  };
}
#endif // HISTOGRAMVIEWERTEST_H
