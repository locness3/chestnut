#include "colorscopewidget.h"

#include <QPainter>
#include <QPen>

using ui::ColorScopeWidget;

constexpr int HORIZONTAL_STEP = 8;
constexpr int PEN_ALPHA = 24;

ColorScopeWidget::ColorScopeWidget(QWidget *parent) : QWidget(parent)
{

}

/**
 * @brief Update the image used to draw scope
 * @param img Image to be drawn from
 */
void ColorScopeWidget::updateImage(QImage img)
{
  img_ = std::move(img);
}


void ColorScopeWidget::paintEvent(QPaintEvent*/*event*/)
{
  QPainter painter(this);
  QPen r_pen(QColor(255,0,0,PEN_ALPHA));
  QPen g_pen(QColor(0,255,0,PEN_ALPHA));
  QPen b_pen(QColor(0,0,255,PEN_ALPHA));
  painter.setRenderHint(QPainter::Antialiasing);
  // clear last paint
  painter.eraseRect(0, 0, width(), img_.height());

  const auto w_step = img_.width() / width();

  // far too slow to be done for every pixel
  for (auto w = 0; w < width(); ++w) {
    for (auto h = 0; h < img_.height(); h+=HORIZONTAL_STEP) {
      // draw pixel value (per channel) on y-axis at x-position
      auto val = img_.pixel(w * w_step, h);
      auto r_pos = qRed(val);
      auto g_pos = qGreen(val);
      auto b_pos = qBlue(val);
      painter.setPen(r_pen);
      painter.drawPoint(w, height() - r_pos);
      painter.setPen(g_pen);
      painter.drawPoint(w, height() - g_pos);
      painter.setPen(b_pen);
      painter.drawPoint(w, height() - b_pos);
    }
  }

  // paint surrounding box
  QPen pen(Qt::black);
  painter.setPen(pen);
  painter.drawRect(0, 0, width(), height()-1);

  //FIXME: do this in 1 loop and don't paint minor over major
  // major lines
  pen.setColor(QColor(0,0,0, 128));
  painter.setPen(pen);
  int h_step = height() / 4;
  for (auto h = h_step; h < height(); h+=h_step) {
    painter.drawLine(0, height() - h, width(), height() -  h);
  }

  // minor lines
  QVector<qreal> dashes;
  dashes << 3 << 3;
  pen.setDashPattern(dashes);
  painter.setPen(pen);
  h_step = height() / 8;
  for (auto h = h_step; h < height(); h+=h_step) {
    painter.drawLine(0, height() - h, width(), height() -  h);
  }

}
