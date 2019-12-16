/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PANELMANAGER_H
#define PANELMANAGER_H

#include "panels/histogramviewer.h"
#include "panels/scopeviewer.h"
#include "panels/grapheditor.h"
#include "panels/timeline.h"
#include "panels/effectcontrols.h"
#include "panels/project.h"
#include "panels/viewer.h"
#include "ui/Forms/markersviewer.h"

void scroll_to_frame_internal(QScrollBar* bar, const int64_t frame, const double zoom, const int area_width);

namespace panels {

  class PanelManager
  {
    public:
      // eh, really a singleton/factory
      PanelManager() = default;
      static void refreshPlayHeaders();

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
      static MarkersViewer& markersViewer();
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
      static MarkersViewer* markers_viewer_;
      static QWidget* parent_;
  };
}

#endif // PANELMANAGER_H
