#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QPen>
#include <QPainter>

namespace ui
{
  class HistogramWidget : public QWidget
  {
      Q_OBJECT
    public:
      explicit HistogramWidget(QWidget *parent = nullptr);
      QVector<int> values_{};
      QColor color_{Qt::white};
    protected:
      void paintEvent(QPaintEvent *event) override;
  };
}
#endif // HISTOGRAMWIDGET_H
