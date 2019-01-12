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
#include "debug.h"

#include <QScrollBar>

Project* e_panel_project = 0;
EffectControls* e_panel_effect_controls = 0;
Viewer* e_panel_sequence_viewer = 0;
Viewer* e_panel_footage_viewer = 0;
Timeline* e_panel_timeline = 0;
GraphEditor* e_panel_graph_editor = 0;

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
	if (e_sequence != NULL) {
		for (int i=0;i<e_sequence->clips.size();i++) {
            ClipPtr clip = e_sequence->clips.at(i);
			if (clip != NULL) {
				for (int j=0;j<e_sequence->selections.size();j++) {
					const Selection& s = e_sequence->selections.at(j);
					bool add = true;
                    if (clip->timeline_info.in >= s.in && clip->timeline_info.out <= s.out && clip->timeline_info.track == s.track) {
						mode = TA_NO_TRANSITION;
					} else if (selection_contains_transition(s, clip, TA_OPENING_TRANSITION)) {
						mode = TA_OPENING_TRANSITION;
					} else if (selection_contains_transition(s, clip, TA_CLOSING_TRANSITION)) {
						mode = TA_CLOSING_TRANSITION;
					} else {
						add = false;
					}

					if (add) {
                        if (clip->timeline_info.track < 0 && vclip == -1) {
							vclip = i;
                        } else if (clip->timeline_info.track >= 0 && aclip == -1) {
							aclip = i;
						} else {
							vclip = -2;
							aclip = -2;
							multiple = true;
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
                ClipPtr vclip_ref = e_sequence->clips.at(vclip);
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

    bool same = (selected_clips.size() == e_panel_effect_controls->selected_clips.size());
	if (same) {
		for (int i=0;i<selected_clips.size();i++) {
            if (selected_clips.at(i) != e_panel_effect_controls->selected_clips.at(i)) {
				same = false;
				break;
			}
		}
	}

    if (e_panel_effect_controls->multiple != multiple || !same) {
        e_panel_effect_controls->multiple = multiple;
        e_panel_effect_controls->set_clips(selected_clips, mode);
	}
}

void update_ui(bool modified) {
	if (modified) {
		update_effect_controls();
	}
    e_panel_effect_controls->update_keyframes();
    e_panel_timeline->repaint_timeline();
    e_panel_sequence_viewer->update_viewer();
    e_panel_graph_editor->update_panel();
}

QDockWidget *get_focused_panel() {
	QDockWidget* w = NULL;
	if (e_config.hover_focus) {
        if (e_panel_project->underMouse()) {
            w = e_panel_project;
        } else if (e_panel_effect_controls->underMouse()) {
            w = e_panel_effect_controls;
        } else if (e_panel_sequence_viewer->underMouse()) {
            w = e_panel_sequence_viewer;
        } else if (e_panel_footage_viewer->underMouse()) {
            w = e_panel_footage_viewer;
        } else if (e_panel_timeline->underMouse()) {
            w = e_panel_timeline;
		}
	}
	if (w == NULL) {
        if (e_panel_project->is_focused()) {
            w = e_panel_project;
        } else if (e_panel_effect_controls->keyframe_focus() || e_panel_effect_controls->is_focused()) {
            w = e_panel_effect_controls;
        } else if (e_panel_sequence_viewer->is_focused()) {
            w = e_panel_sequence_viewer;
        } else if (e_panel_footage_viewer->is_focused()) {
            w = e_panel_footage_viewer;
        } else if (e_panel_timeline->focused()) {
            w = e_panel_timeline;
		}
	}
	return w;
}

void alloc_panels(QWidget* parent) {
	// TODO maybe replace these with non-pointers later on?
    e_panel_sequence_viewer = new Viewer(parent);
    e_panel_sequence_viewer->setObjectName("seq_viewer");
    e_panel_footage_viewer = new Viewer(parent);
    e_panel_footage_viewer->setObjectName("footage_viewer");
    e_panel_project = new Project(parent);
    e_panel_project->setObjectName("proj_root");
    e_panel_effect_controls = new EffectControls(parent);
    init_effects();
    e_panel_effect_controls->setObjectName("fx_controls");
    e_panel_timeline = new Timeline(parent);
    e_panel_timeline->setObjectName("timeline");
    e_panel_graph_editor = new GraphEditor(parent);
    e_panel_graph_editor->setObjectName("graph_editor");
}

void free_panels() {
    delete e_panel_sequence_viewer;
    e_panel_sequence_viewer = NULL;
    delete e_panel_footage_viewer;
    e_panel_footage_viewer = NULL;
    delete e_panel_project;
    e_panel_project = NULL;
    delete e_panel_effect_controls;
    e_panel_effect_controls = NULL;
    delete e_panel_timeline;
    e_panel_timeline = NULL;
}

void scroll_to_frame_internal(QScrollBar* bar, long frame, double zoom, int area_width) {
	int screen_point = getScreenPointFromFrame(zoom, frame) - bar->value();
	int min_x = area_width*0.1;
	int max_x = area_width-min_x;
	if (screen_point < min_x) {
		bar->setValue(getScreenPointFromFrame(zoom, frame) - min_x);
	} else if (screen_point > max_x) {
		bar->setValue(getScreenPointFromFrame(zoom, frame) - max_x);
	}
}
