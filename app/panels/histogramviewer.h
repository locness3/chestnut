#ifndef HISTOGRAMVIEWER_H
#define HISTOGRAMVIEWER_H

#include <QDockWidget>
#include <QVBoxLayout>
#include <QToolButton>

#include "ui/histogramwidget.h"

namespace panels
{

  class HistogramViewer : public QDockWidget
  {
      Q_OBJECT
    public:
      explicit HistogramViewer(QWidget* parent = nullptr);

    public slots:
      /**
       * @brief On the receipt of a grabbed frame from the renderer generate histograms per channel
       * @param img Image of the final, rendered frame, minus the gizmos
       */
      void frameGrabbed(const QImage& img);

    private:
      friend class HistogramViewerTest;
      QVBoxLayout* layout_{nullptr};
      ui::HistogramWidget* histogram_{nullptr};
      ui::HistogramWidget* histogram_red_{nullptr};
      ui::HistogramWidget* histogram_green_{nullptr};
      ui::HistogramWidget* histogram_blue_{nullptr};
      QToolButton* gray_button_{nullptr};
      QToolButton* red_button_{nullptr};
      QToolButton* green_button_{nullptr};
      QToolButton* blue_button_{nullptr};
      QToolButton* clip_button_{nullptr};
      bool full_resolution_{false};

      /*
       * Populate this viewer with its widgets
       */
      void setup();

    signals:
      void drawClippedPixels(const bool state);

    private slots:
      /*
       * Handle a checkable button state change
       */
      void buttonChecked(bool checked);
  };

}

#endif // HISTOGRAMVIEWER_H
