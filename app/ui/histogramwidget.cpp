#include "histogramwidget.h"

#include <QtGlobal>

using ui::HistogramWidget;

constexpr int MAX_PIXELS = 256;

HistogramWidget::HistogramWidget(QWidget *parent)
  : QWidget(parent)
{
  values_.fill(0, MAX_PIXELS);
}



void HistogramWidget::paintEvent(QPaintEvent */*event*/)
{
  if (!isVisible()) {
    return;
  }
  auto max_val = 0; // for scaling results
  //FIXME: find a better way around this
  for (auto i = 1; i < 254; ++i) { //exclude max + min. skewing result
    if (values_[i] > max_val) {
      max_val = values_[i];
    }
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  QPen pen;
  const auto h = height();

  if (max_val > 0) {
    // paint histogram
    pen.setColor(color_);
    painter.setPen(pen);
    double scaled = 0.0;
    int32_t j;
    for (int i = 0; i < MAX_PIXELS; ++i) {
      scaled = static_cast<double>(values_[i]) / max_val;
      scaled = qMin(scaled, static_cast<double>(h));
      j = static_cast<int32_t>(h - qRound(scaled * h));
      painter.drawLine(i+1, h, i+1, j);
    }
  } else {
    painter.eraseRect(1, 1, width() - 1, height() - 1);
  }

  // paint surrounding box
  pen.setColor(Qt::black);
  painter.setPen(pen);
  painter.drawRect(0, 0, MAX_PIXELS, h-1);

  // paint grid
  QVector<qreal> dashes;
  dashes << 3 << 3;
  pen.setDashPattern(dashes);
  QColor clr = pen.color();
  clr.setAlpha(128);
  pen.setColor(clr);
  painter.setPen(pen);
  painter.drawLine(0, h/2, MAX_PIXELS, h/2);
  painter.drawLine(MAX_PIXELS/2, 0, MAX_PIXELS/2, h);
}
