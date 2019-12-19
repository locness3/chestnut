#include "rgbparade.h"

using chestnut::panels::RGBParade;

constexpr auto TITLE = "RGB Parade Viewer";
constexpr auto IMAGE_WIDTH = 720;

RGBParade::RGBParade(QWidget* parent)
  : QDockWidget(parent),
    widget_(new ui::RGBParadeWidget(this))
{
  setup();
}

void RGBParade::frameGrabbed(const QImage& img)
{
  Q_ASSERT(widget_);
  if (img.width() < IMAGE_WIDTH) {
    widget_->updateImage(img);
  } else {
    const auto ratio = static_cast<double>(img.width() / IMAGE_WIDTH);
    widget_->updateImage(img.scaled(IMAGE_WIDTH, qRound(static_cast<double>(img.height()/ ratio))));
  }
  widget_->update();
}


void RGBParade::setup()
{
  setWindowTitle(tr(TITLE));
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  auto main_widget = new QWidget();
  setWidget(main_widget);
  layout_ = new QVBoxLayout();
  main_widget->setLayout(layout_);
  layout_->addWidget(widget_);
}
