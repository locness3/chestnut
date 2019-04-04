#ifndef COLORSCOPEWIDGET_H
#define COLORSCOPEWIDGET_H

#include <QWidget>

namespace ui {

  class ColorScopeWidget : public QWidget
  {
      Q_OBJECT
    public:
      explicit ColorScopeWidget(QWidget *parent = nullptr);
      /**
       * @brief Update the image used to draw scope
       * @param img Image to be drawn from
       */
      void updateImage(QImage img);
    protected:
      void paintEvent(QPaintEvent *event) override;
    private:
      QImage img_;
  };
}

#endif // COLORSCOPEWIDGET_H
