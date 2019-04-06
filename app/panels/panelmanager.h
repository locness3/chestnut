#ifndef PANELMANAGER_H
#define PANELMANAGER_H


#include "panels/histogramviewer.h"
#include "panels/scopeviewer.h"
#include "panels/grapheditor.h"
#include "panels/timeline.h"
#include "panels/effectcontrols.h"
#include "panels/project.h"
#include "panels/viewer.h"

void scroll_to_frame_internal(QScrollBar* bar, const long frame, const double zoom, const int area_width);

namespace panels {

  class PanelManager
  {
    public:
      // eh, really a singleton/factory
      PanelManager() = default;

      static void refreshPanels(const bool modified);
      static QDockWidget* getFocusedPanel();

      static bool setParent(QWidget* parent);
      static HistogramViewer& histogram();
      static ScopeViewer& colorScope();
      static GraphEditor& graphEditor();
      static Timeline& timeLine();
      static EffectControls& fxControls();
      static Project& projectViewer();
      static Viewer& sequenceViewer();
      static Viewer& footageViewer();
      static void tearDown();
    private:
      // cannot use smart_ptrs with QObject::connect. at least not reliably (double-frees)
      static HistogramViewer* histogram_viewer_;
      static ScopeViewer* scope_viewer_;
      static GraphEditor* graph_editor_;
      static Timeline* timeline_;
      static EffectControls* fx_controls_;
      static Project* project_;
      static Viewer* sequence_viewer_;
      static Viewer* footage_viewer_;
      static QWidget* parent_;
  };
}

#endif // PANELMANAGER_H
