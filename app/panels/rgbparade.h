#ifndef RGBPARADE_H
#define RGBPARADE_H

#include <QDockWidget>
#include <QVBoxLayout>

#include "ui/rgbparadewidget.h"

namespace chestnut::panels
{
  class RGBParade : public QDockWidget
  {
    public:
      explicit RGBParade(QWidget* parent=nullptr);
    public slots:
      /**
       * @brief On the receipt of a grabbed frame from the renderer generate histograms per channel
       * @param img Image of the final, rendered frame, minus the gizmos
       */
      void frameGrabbed(const QImage& img);
    private:
      QVBoxLayout* layout_{nullptr};
      ui::RGBParadeWidget* widget_ {nullptr};

      void setup();
  };
}



#endif // RGBPARADE_H
