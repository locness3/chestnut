#ifndef SCOPEVIEWER_H
#define SCOPEVIEWER_H

#include <QDockWidget>

#include "ui/colorscopewidget.h"

namespace panels
{
  class ScopeViewer : public QDockWidget
  {
      Q_OBJECT
    public:
      explicit ScopeViewer(QWidget* parent = nullptr);

    public slots:
      /**
       * @brief On the receipt of a grabbed frame from the renderer generate scopes
       * @param img Image of the final, rendered frame, minus the gizmos
       */
      void frameGrabbed(const QImage& img);

    private:
      ui::ColorScopeWidget* color_scope_{nullptr};

      /*
       * Populate this viewer with its widgets
       */
      void setup();
  };
}

#endif // SCOPEVIEWER_H
