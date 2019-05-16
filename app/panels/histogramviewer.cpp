#include "histogramviewer.h"

#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QImage>
#include <QToolButton>

#include "debug.h"

using panels::HistogramViewer;

constexpr int HIST_MIN_WIDTH = 260;
constexpr int HIST_MIN_HEIGHT = 100;
constexpr int HIST_COLOR_ALPHA = 128;

constexpr int PANEL_WIDTH = 272;
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
  if ( (histogram_ == nullptr) || (histogram_red_ == nullptr)
       || (histogram_green_ == nullptr) || (histogram_blue_ == nullptr) ) {
    qCritical() << "Histogram instance is NULL";
    return;
  }

  // build histogram
  // clear previous values
  histogram_->values_.fill(0);
  histogram_red_->values_.fill(0);
  histogram_green_->values_.fill(0);
  histogram_blue_->values_.fill(0);

  // a full-resolution histogram shouldn't be done during playback
  const auto div = full_resolution_ ? 1 : HIST_GEN_STEP; //TODO: create a way to enable full_resolution
  QRgb clr;
  for (auto w = 0; w < img.width(); w+=div){
    for (auto h = 0; h < img.height(); h+=div) {
      clr = img.pixel(w, h);
      histogram_->values_.at(static_cast<size_t>(qGray(clr)))++;
      histogram_red_->values_.at(static_cast<size_t>(qRed(clr)))++;
      histogram_green_->values_.at(static_cast<size_t>(qGreen(clr)))++;
      histogram_blue_->values_.at(static_cast<size_t>(qBlue(clr)))++;
    }
  }
  // Frame may have been retrieved after an effect change
  histogram_->update();
  histogram_red_->update();
  histogram_green_->update();
  histogram_blue_->update();
}

/*
 * Populate this viewer with its widgets
 */
void HistogramViewer::setup()
{
  setWindowTitle(tr("Histogram Viewer"));
  resize(PANEL_WIDTH, PANEL_HEIGHT);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto main_widget = new QWidget();
  setWidget(main_widget);

  layout_ = new QVBoxLayout();
  main_widget->setLayout(layout_);
  auto button_layout = new QHBoxLayout();
  layout_->addLayout(button_layout);

  gray_button_ = new QToolButton();
  gray_button_->setCheckable(true);
  gray_button_->setChecked(true);
  gray_button_->setText("All");
  connect(gray_button_, &QToolButton::clicked, this, &HistogramViewer::buttonChecked);
  button_layout->addWidget(gray_button_);

  red_button_ = new QToolButton();
  red_button_->setCheckable(true);
  red_button_->setChecked(true);
  red_button_->setText("R");
  connect(red_button_, &QToolButton::clicked, this, &HistogramViewer::buttonChecked);
  button_layout->addWidget(red_button_);

  green_button_ = new QToolButton();
  green_button_->setCheckable(true);
  green_button_->setChecked(true);
  green_button_->setText("G");
  connect(green_button_, &QToolButton::clicked, this, &HistogramViewer::buttonChecked);
  button_layout->addWidget(green_button_);

  blue_button_ = new QToolButton();
  blue_button_->setCheckable(true);
  blue_button_->setChecked(true);
  blue_button_->setText("B");
  connect(blue_button_, &QToolButton::clicked, this, &HistogramViewer::buttonChecked);
  button_layout->addWidget(blue_button_);

  clip_button_ = new QToolButton();
  clip_button_->setCheckable(true);
  clip_button_->setChecked(false);
  clip_button_->setText("Show Clipped");
  connect(clip_button_, &QToolButton::clicked, this, &HistogramViewer::buttonChecked);
  button_layout->addWidget(clip_button_);

  auto h_spacer = new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  button_layout->addSpacerItem(h_spacer);

  histogram_= new ui::HistogramWidget();
  layout_->addWidget(histogram_);
  histogram_->color_.setAlpha(HIST_COLOR_ALPHA);
  histogram_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  histogram_red_ = new ui::HistogramWidget();
  layout_->addWidget(histogram_red_);
  histogram_red_->color_ = QColor(250, 0, 0, HIST_COLOR_ALPHA);
  histogram_red_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  histogram_green_ = new ui::HistogramWidget();
  layout_->addWidget(histogram_green_);
  histogram_green_->color_ = QColor(0, 250, 0, HIST_COLOR_ALPHA);
  histogram_green_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);

  histogram_blue_ = new ui::HistogramWidget();
  layout_->addWidget(histogram_blue_);
  histogram_blue_->color_ = QColor(0, 0, 250, HIST_COLOR_ALPHA);
  histogram_blue_->setMinimumSize(HIST_MIN_WIDTH, HIST_MIN_HEIGHT);
}

/*
 * Handle a checkable button state change
 */
void HistogramViewer::buttonChecked(bool checked)
{
  if ( (this->sender() == gray_button_) && (histogram_ != nullptr) ) {
    histogram_->setVisible(checked);
  } else if ( (this->sender() == red_button_) && (histogram_red_ != nullptr) ) {
    histogram_red_->setVisible(checked);
  } else if ( (this->sender() == green_button_) && (histogram_green_ != nullptr) ) {
    histogram_green_->setVisible(checked);
  } else if ( (this->sender() == blue_button_) && (histogram_blue_ != nullptr) ) {
    histogram_blue_->setVisible(checked);
  } else if (this->sender() == clip_button_) {
    emit drawClippedPixels(checked);
  } else {
    qWarning() << "Unhandled button check";
  }
}
