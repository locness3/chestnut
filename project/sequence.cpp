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
#include "sequence.h"

#include "clip.h"
#include "transition.h"

#include "debug.h"

Sequence::Sequence() {
}

Sequence::~Sequence() {
	// dealloc all clips
	for (int i=0;i<clips.size();i++) {
		delete clips.at(i);
	}
}

Sequence* Sequence::copy() {
	Sequence* s = new Sequence();
    s->m_name = m_name + " (copy)";
    s->m_dimensions = m_dimensions;
    s->m_frameRate = m_frameRate;
    s->m_audioFrequency = m_audioFrequency;
    s->m_audioLayout = m_audioLayout;
	s->clips.resize(clips.size());
	for (int i=0;i<clips.size();i++) {
		Clip* c = clips.at(i);
		if (c == NULL) {
			s->clips[i] = NULL;
		} else {
			Clip* copy = c->copy(s);
			copy->linked = c->linked;
			s->clips[i] = copy;
		}
	}
	return s;
}

long Sequence::getEndFrame() {
	long end = 0;
	for (int j=0;j<clips.size();j++) {
		Clip* c = clips.at(j);
		if (c != NULL && c->timeline_out > end) {
			end = c->timeline_out;
		}
	}
	return end;
}

void Sequence::hard_delete_transition(Clip *c, int type) {
	int transition_index = (type == TA_OPENING_TRANSITION) ? c->opening_transition : c->closing_transition;
	if (transition_index > -1) {
		bool del = true;

		Transition* t = transitions.at(transition_index);
		if (t->secondary_clip != NULL) {
			for (int i=0;i<clips.size();i++) {
				Clip* comp = clips.at(i);
				if (comp != NULL
						&& c != comp
						&& (c->opening_transition == transition_index
						|| c->closing_transition == transition_index)) {
					if (type == TA_OPENING_TRANSITION) {
						// convert to closing transition
						t->parent_clip = t->secondary_clip;
					}

					del = false;
					t->secondary_clip = NULL;
				}
			}
		}

		if (del) {
			delete transitions.at(transition_index);
			transitions[transition_index] = NULL;
		}

		if (type == TA_OPENING_TRANSITION) {
			c->opening_transition = -1;
		} else {
			c->closing_transition = -1;
		}
	}
}


const QString&  Sequence::getName() const {
    return m_name;
}

void Sequence::setName(const QString& name) {
    m_name = name;
}

const QPair<int,int>& Sequence::getDimensions() const {
    return m_dimensions;
}
void Sequence::setDimensions(const QPair<int,int>& dimensions) {
    m_dimensions = dimensions;
}

double Sequence::getFrameRate() const {
    return m_frameRate;
}
void Sequence::setFrameRate(const double frameRate) {
    m_frameRate = frameRate;
}


int Sequence::getAudioFrequency() const {
    return m_audioFrequency;
}
void Sequence::setAudioFrequency(const int audioFrequency) {
    m_audioFrequency = audioFrequency;
}


int Sequence::getAudioLayout() const {
    return m_audioLayout;
}
void Sequence::setAudioLayout(const int audioLayout) {
    m_audioLayout = audioLayout;
}
void Sequence::getTrackLimits(int* video_tracks, int* audio_tracks) {
	int vt = 0;
	int at = 0;
	for (int j=0;j<clips.size();j++) {
		Clip* c = clips.at(j);
		if (c != NULL) {
			if (c->track < 0 && c->track < vt) { // video clip
				vt = c->track;
			} else if (c->track > at) {
				at = c->track;
			}
		}
	}
	if (video_tracks != NULL) *video_tracks = vt;
	if (audio_tracks != NULL) *audio_tracks = at;
}

// static variable for the currently active sequence
Sequence* sequence = NULL;
