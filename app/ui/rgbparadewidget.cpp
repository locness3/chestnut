#include "rgbparadewidget.h"

using chestnut::ui::RGBParadeWidget;

constexpr auto PEN_ALPHA = 24;

namespace  {
  const QPen r_pen(QColor(255, 0, 0, PEN_ALPHA));
  const QPen g_pen(QColor(0, 255, 0, PEN_ALPHA));
  const QPen b_pen(QColor(0, 0, 255, PEN_ALPHA));
}

RGBParadeWidget::RGBParadeWidget(QWidget *parent) : QWidget(parent)
{

}

void RGBParadeWidget::drawBackground(QPainter& p) const
{
  QPen pen(Qt::black);
  p.setPen(pen);
  p.drawRect(0, 0, COLORS_PER_CHANNEL * 3, height() - 1);
  p.drawLine(256, 0, 256, height());
  p.drawLine(512, 0, 512, height());
}


void RGBParadeWidget::updateImage(QImage img)
{
  image_ = std::move(img);
}


void RGBParadeWidget::paintEvent(QPaintEvent *event)
{
  if (!isVisible()) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
//  drawBackground(painter);
  QPen pen(Qt::red);
  const auto w_step = static_cast<double>(image_.width()) / width();
  const auto h_step = static_cast<double>(height()) / 256;
  QRgb val;
  const auto part = qRound(static_cast<double>(width()) / 3.0);
  for (auto w = 0; w < width(); ++w) {
    for (auto h = 0; h < image_.height(); ++h) {
      val = image_.pixel(static_cast<int32_t>(qRound(w * w_step)), h);
      // red
      const auto r_x = qRound(static_cast<double>(w)/3.0);
      painter.setPen(r_pen);
      painter.drawPoint(r_x, height() - static_cast<int32_t>(qRound(qRed(val) * h_step)));
      // green
      const auto g_x = r_x + part;
      painter.setPen(g_pen);
      painter.drawPoint(g_x, height() - static_cast<int32_t>(qRound(qGreen(val) * h_step)));
      // blue
      const auto b_x = r_x + part + part;
      painter.setPen(b_pen);
      painter.drawPoint(b_x, height() - static_cast<int32_t>(qRound(qBlue(val) * h_step)));
    }
  }
}
