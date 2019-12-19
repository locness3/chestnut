#ifndef RGBPARADEWIDGET_H
#define RGBPARADEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <array>

namespace chestnut::ui
{
  constexpr int COLORS_PER_CHANNEL = 256; // QImage can only support up to 8-bit per channel
  class RGBParadeWidget : public QWidget
  {
      Q_OBJECT
    public:
      explicit RGBParadeWidget(QWidget *parent = nullptr);
      void updateImage(QImage img);
    protected:
      void paintEvent(QPaintEvent *event) override;

    private:
      QImage image_;
      void drawBackground(QPainter& p) const;
      void drawValues(QPainter& p, QPen& pen, const std::array<int, COLORS_PER_CHANNEL>& values, const int offset);
  };
}



#endif // RGBPARADEWIDGET_H
