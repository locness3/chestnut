#ifndef PANELMANAGER_H
#define PANELMANAGER_H


#include "panels/histogramviewer.h"
#include "panels/scopeviewer.h"
#include "panels/grapheditor.h"
#include "panels/timeline.h"
#include "panels/effectcontrols.h"

namespace panels {

  class PanelManager
  {
    public:
      // eh, really a singleton/factory
      PanelManager() = default;

      static bool setParent(QWidget* parent);
      static HistogramViewer& histogram();
      static ScopeViewer& colorScope();
      static GraphEditor& graphEditor();
      static Timeline& timeLine();
      static EffectControls& fxControls();
      static void tearDown();
    private:
      // cannot use smart_ptrs with QObject::connect. at least not reliably (double-frees)
      static HistogramViewer* histogram_viewer_;
      static ScopeViewer* scope_viewer_;
      static GraphEditor* graph_editor_;
      static Timeline* timeline_;
      static EffectControls* fx_controls_;
      static QWidget* parent_;
  };
}

#endif // PANELMANAGER_H
