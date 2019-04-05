/*
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
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
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "panels.h"

#include "timeline.h"
#include "effectcontrols.h"
#include "viewer.h"
#include "project.h"
#include "project/sequence.h"
#include "project/clip.h"
#include "project/transition.h"
#include "io/config.h"
#include "grapheditor.h"
#include "histogramviewer.h"
#include "scopeviewer.h"
#include "debug.h"
#include "panels/panelmanager.h"

#include <QScrollBar>
#include <QCoreApplication>

Project* e_panel_project = nullptr;
Viewer* e_panel_sequence_viewer = nullptr;
Viewer* e_panel_footage_viewer = nullptr;

using panels::PanelManager;

void update_effect_controls() {
  // SEND CLIPS TO EFFECT CONTROLS
  // find out how many clips are selected
  // limits to one video clip and one audio clip and only if they're linked
  // one of these days it might be nice to have multiple clips in the effects panel
  bool multiple = false;
  int vclip = -1;
  int aclip = -1;
  QVector<int> selected_clips;
  int mode = TA_NO_TRANSITION;
  if (global::sequence != nullptr) {
    for (int i=0;i<global::sequence->clips_.size();i++) {
      ClipPtr clip = global::sequence->clips_.at(i);
      if (clip != nullptr) {
        for (int j=0;j<global::sequence->selections_.size();j++) {
          const Selection& s = global::sequence->selections_.at(j);
          bool add = true;
          if (clip->timeline_info.in >= s.in && clip->timeline_info.out <= s.out && clip->timeline_info.track_ == s.track) {
            mode = TA_NO_TRANSITION;
          } else if (selection_contains_transition(s, clip, TA_OPENING_TRANSITION)) {
            mode = TA_OPENING_TRANSITION;
          } else if (selection_contains_transition(s, clip, TA_CLOSING_TRANSITION)) {
            mode = TA_CLOSING_TRANSITION;
          } else {
            add = false;
          }

          if (add) {
            if (clip->timeline_info.isVideo() && vclip == -1) {
              vclip = i;
            } else if (clip->timeline_info.track_ >= 0 && aclip == -1) {
              aclip = i;
            } else {
              vclip = -2;
              aclip = -2;
              multiple = true;
              break;
            }
          }
        }
      }
    }

    if (!multiple) {
      // check if aclip is linked to vclip
      if (vclip >= 0) selected_clips.append(vclip);
      if (aclip >= 0) selected_clips.append(aclip);
      if (vclip >= 0 && aclip >= 0) {
        bool found = false;
        ClipPtr vclip_ref = global::sequence->clips_.at(vclip);
        for (int i=0;i<vclip_ref->linked.size();i++) {
          if (vclip_ref->linked.at(i) == aclip) {
            found = true;
            break;
          }
        }
        if (!found) {
          // only display multiple clips if they're linked
          selected_clips.clear();
          multiple = true;
        }
      }
    }
  }

  bool same = (selected_clips.size() == PanelManager::fxControls().selected_clips.size());
  if (same) {
    for (int i=0;i<selected_clips.size();i++) {
      if (selected_clips.at(i) != PanelManager::fxControls().selected_clips.at(i)) {
        same = false;
        break;
      }
    }
  }

  if (PanelManager::fxControls().multiple != multiple || !same) {
    PanelManager::fxControls().multiple = multiple;
    PanelManager::fxControls().set_clips(selected_clips, mode);
  }
}

void update_ui(const bool modified)
{
  if (modified) {
    update_effect_controls();
  }
  PanelManager::fxControls().update_keyframes();
  PanelManager::timeLine().repaint_timeline();
  e_panel_sequence_viewer->update_viewer();
  PanelManager::graphEditor().update_panel();
}

QDockWidget* get_focused_panel()
{
  QDockWidget* w = nullptr;
  if (e_config.hover_focus) {
    if (e_panel_project->underMouse()) {
      w = e_panel_project;
    } else if (PanelManager::fxControls().underMouse()) {
      w = &PanelManager::fxControls();
    } else if (e_panel_sequence_viewer->underMouse()) {
      w = e_panel_sequence_viewer;
    } else if (e_panel_footage_viewer->underMouse()) {
      w = e_panel_footage_viewer;
    } else if (PanelManager::timeLine().underMouse()) {
      w = &PanelManager::timeLine();
    } else if (PanelManager::graphEditor().view_is_under_mouse()) {
      w = &PanelManager::graphEditor();
    } else if (PanelManager::histogram().underMouse()) {
      w = &PanelManager::histogram();
    } else if (PanelManager::colorScope().underMouse()) {
      w = &PanelManager::colorScope();
    }
  }

  if (w == nullptr) {
    if (e_panel_project->is_focused()) {
      w = e_panel_project;
    } else if (PanelManager::fxControls().keyframe_focus() || PanelManager::fxControls().is_focused()) {
      w = &PanelManager::fxControls();
    } else if (e_panel_sequence_viewer->is_focused()) {
      w = e_panel_sequence_viewer;
    } else if (e_panel_footage_viewer->is_focused()) {
      w = e_panel_footage_viewer;
    } else if (PanelManager::timeLine().focused()) {
      w = &PanelManager::timeLine();
    } else if (PanelManager::graphEditor().view_is_focused()) {
      w = &PanelManager::graphEditor();
    }
  }
  return w;
}

void alloc_panels(QWidget* parent)
{
  // TODO maybe replace these with non-pointers later on?
  e_panel_sequence_viewer = new Viewer(parent);
  e_panel_sequence_viewer->setObjectName("seq_viewer");
  e_panel_sequence_viewer->set_panel_name(QCoreApplication::translate("Viewer", "Sequence Viewer"));
  e_panel_footage_viewer = new Viewer(parent);
  e_panel_footage_viewer->setObjectName("footage_viewer");
  e_panel_footage_viewer->set_panel_name(QCoreApplication::translate("Viewer", "Media Viewer"));
  e_panel_project = new Project(parent);
  e_panel_project->setObjectName("proj_root");
//  e_panel_effect_controls = new EffectControls(parent);
//  init_effects();
}

void free_panels()
{
  delete e_panel_sequence_viewer;
  e_panel_sequence_viewer = nullptr;
  delete e_panel_footage_viewer;
  e_panel_footage_viewer = nullptr;
  delete e_panel_project;
  e_panel_project = nullptr;
}

void scroll_to_frame_internal(QScrollBar* bar, long frame, double zoom, int area_width)
{
  const int screen_point = getScreenPointFromFrame(zoom, frame) - bar->value();
  const auto min_x = static_cast<int>(area_width * 0.1);
  const int max_x = area_width - min_x;
  if (screen_point < min_x) {
    bar->setValue(getScreenPointFromFrame(zoom, frame) - min_x);
  } else if (screen_point > max_x) {
    bar->setValue(getScreenPointFromFrame(zoom, frame) - max_x);
  }
}
