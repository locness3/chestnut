#include "histogramviewertest.h"
#include "panels/histogramviewer.h"

#include <QtTest/QTest>

using panels::HistogramViewerTest;

HistogramViewerTest::HistogramViewerTest()
{

}



void HistogramViewerTest::testCaseSetup()
{
  panels::HistogramViewer vwr;

  vwr.setup();
  QVERIFY(vwr.layout_ != nullptr);
  QVERIFY(vwr.histogram_ != nullptr);
  QVERIFY(vwr.histogram_->values_.size() == 256);
  QVERIFY(vwr.histogram_red_ != nullptr);
  QVERIFY(vwr.histogram_red_->values_.size() == 256);
  QVERIFY(vwr.histogram_green_ != nullptr);
  QVERIFY(vwr.histogram_green_->values_.size() == 256);
  QVERIFY(vwr.histogram_blue_ != nullptr);
  QVERIFY(vwr.histogram_blue_->values_.size() == 256);
  QVERIFY(vwr.full_resolution_ == false);
}
