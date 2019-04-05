#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QPen>
#include <QPainter>
#include <array>

constexpr int COLORS_PER_CHANNEL = 256; // QImage can only support up to 8-bit per channel

namespace ui
{
  class HistogramWidget : public QWidget
  {
      Q_OBJECT
    public:
      explicit HistogramWidget(QWidget *parent = nullptr);
      std::array<int, COLORS_PER_CHANNEL> values_{};
      QColor color_{Qt::white};
    protected:
      void paintEvent(QPaintEvent *event) override;
  };
}
#endif // HISTOGRAMWIDGET_H
