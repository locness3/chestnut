#include "histogramviewer.h"

#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QImage>

#include "debug.h"

using panels::HistogramViewer;

constexpr int HIST_MIN_WIDTH = 260;
constexpr int HIST_MIN_HEIGHT = 100;
constexpr int HIST_COLOR_ALPHA = 200;

constexpr int PANEL_WIDTH = 300;
constexpr int PANEL_HEIGHT = 480;

constexpr int HIST_GEN_STEP = 24;


HistogramViewer::HistogramViewer(QWidget* parent) : QDockWidget(parent)
{
  setup();
}

/**
 * @brief On the receipt of a grabbed frame from the renderer
 * @param img Image of the final, rendered frame, minus the gizmos
 */
void HistogramViewer::frameGrabbed(const QImage& img)
{
  if (histogram_ == nullptr || histogram_red_ == nullptr || histogram_green_ == nullptr || histogram_blue_ == nullptr) {
    qCritical() << "Histogram instance is NULL";
    return;
  }

  // build histogram
  // clear previous values
  histogram_->values_.fill(0, 256);
  histogram_red_->values_.fill(0, 256);
  histogram_green_->values_.fill(0, 256);
  histogram_blue_->values_.fill(0, 256);

  // a full-resolution histogram shouldn't be done during playback
  const auto div = full_resolution_ ? 1 : HIST_GEN_STEP; //TODO: create a way to enable full_resolution
  for (auto w = 0; w < img.width(); w+=div){
    for (auto h = 0; h < img.height(); h+=div) {
      const QRgb clr = img.pixel(w, h);
      histogram_->values_[qGray(clr)] += 1;
      histogram_red_->values_[qRed(clr)] += 1;
      histogram_green_->values_[qGreen(clr)] += 1;
      histogram_blue_->values_[qBlue(clr)] += 1;
    }
  }
}

/*
 * Populate this viewer with its widgets
 */
void HistogramViewer::setup()
{
  //TODO: add buttons to disable/enable channels
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  setWindowTitle(tr("Histogram Viewer"));
  resize(PANEL_WIDTH, PANEL_HEIGHT);
  auto main_widget = new QWidget();
  setWidget(main_widget);

  layout_ = new QVBoxLayout();
  main_widget->setLayout(layout_);

  histogram_= new ui::HistogramWidget();
  layout_->addWidget(histogram_);
  histogram_->color_.setAlpha(HIST_COLOR_ALPHA);
  histogram_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  histogram_red_ = new ui::HistogramWidget();
  layout_->addWidget(histogram_red_);
  histogram_red_->color_ = Qt::red;
  histogram_red_->color_.setAlpha(HIST_COLOR_ALPHA);
  histogram_red_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  histogram_green_ = new ui::HistogramWidget();
  layout_->addWidget(histogram_green_);
  histogram_green_->color_ = Qt::green;
  histogram_green_->color_.setAlpha(HIST_COLOR_ALPHA);
  histogram_green_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  histogram_blue_ = new ui::HistogramWidget();
  layout_->addWidget(histogram_blue_);
  histogram_blue_->color_ = Qt::blue;
  histogram_blue_->color_.setAlpha(HIST_COLOR_ALPHA);
  histogram_blue_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  auto spacer = new QSpacerItem(20,20, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout_->addSpacerItem(spacer);
}
