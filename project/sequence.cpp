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

#include <QCoreApplication>

#include "clip.h"
#include "transition.h"

#include "debug.h"

Sequence::Sequence() : ProjectItem()
{
}


Sequence::~Sequence() {
}

/**
 * @brief Copy constructor
 * @param cpy
 */
Sequence::Sequence(const Sequence& cpy) :
    selections(cpy.selections),
    clips(cpy.clips),
    save_id(cpy.save_id),
    using_workarea(cpy.using_workarea),
    enable_workarea(cpy.enable_workarea),
    workarea_in(cpy.workarea_in),
    workarea_out(cpy.workarea_out),
    transitions(cpy.transitions),
    markers(cpy.markers),
    playhead(cpy.playhead),
    wrapper_sequence(cpy.wrapper_sequence),
    width(cpy.width),
    height(cpy.height),
    frame_rate(cpy.frame_rate),
    audio_frequency(cpy.audio_frequency),
    audio_layout(cpy.audio_layout)
{

}

std::shared_ptr<Sequence> Sequence::copy() {
    std::shared_ptr<Sequence> s = std::make_shared<Sequence>();
    s->name = QCoreApplication::translate("Sequence", "%1 (copy)").arg(name);
    s->width = width;
    s->height = height;
    s->frame_rate = frame_rate;
    s->audio_frequency = audio_frequency;
    s->audio_layout = audio_layout;
    s->clips.resize(clips.size());

    for (int i=0;i<clips.size();i++) {
        ClipPtr c = clips.at(i);
        if (c == nullptr) {
            s->clips[i] = nullptr;
        } else {
            ClipPtr copy(c->copy(s));
            copy->linked = c->linked;
            s->clips[i] = copy;
        }
    }
    return s;
}

long Sequence::getEndFrame() const{
    long end = 0;
    for (int j=0;j<clips.size();j++) {
        ClipPtr c = clips.at(j);
        if (c != nullptr && c->timeline_info.out > end) {
            end = c->timeline_info.out;
        }
    }
    return end;
}

void Sequence::hard_delete_transition(ClipPtr& c, const int type) {
    int transition_index = (type == TA_OPENING_TRANSITION) ? c->opening_transition : c->closing_transition;
    if (transition_index > -1) {
        bool del = true;

        auto t = transitions.at(transition_index);
        if (!t->secondary_clip.expired()) {
            for (int i=0;i<clips.size();i++) {
                ClipPtr comp = clips.at(i);
                if (comp != nullptr && c != comp
                        && (c->opening_transition == transition_index || c->closing_transition == transition_index)) {
                    if (type == TA_OPENING_TRANSITION) {
                        // convert to closing transition
                        t->parent_clip = t->secondary_clip.lock();
                    }

                    del = false;
                    t->secondary_clip.reset();
                }
            }
        }

        if (del) {
            transitions[transition_index] = nullptr;
        }

        if (type == TA_OPENING_TRANSITION) {
            c->opening_transition = -1;
        } else {
            c->closing_transition = -1;
        }
    }
}


void Sequence::setWidth(const int val) {
    width = val;
}
int Sequence::getWidth() const {
    return width;
}

void Sequence::setHeight(const int val) {
    height = val;
}
int Sequence::getHeight() const {
    return height;
}

double Sequence::getFrameRate() const {
    return frame_rate;
}
void Sequence::setFrameRate(const double frameRate) {
    frame_rate = frameRate;
}


int Sequence::getAudioFrequency() const {
    return audio_frequency;
}
void Sequence::setAudioFrequency(const int frequency) {
    audio_frequency = frequency;
}


int Sequence::getAudioLayout() const {
    return audio_layout;
}
void Sequence::setAudioLayout(const int layout) {
    audio_layout = layout;
}
void Sequence::getTrackLimits(int& video_limit, int& audio_limit) const {
    video_limit = 0;
    audio_limit = 0;
    for (int j=0;j<clips.size();j++) {
        ClipPtr c = clips.at(j);
        if (c != nullptr) {
            if (c->timeline_info.track < 0 && c->timeline_info.track < video_limit) { // video clip
                video_limit = c->timeline_info.track;
            } else if (c->timeline_info.track > audio_limit) {
                audio_limit = c->timeline_info.track;
            }
        }
    }//for
}

// static variable for the currently active sequence
SequencePtr global::sequence;
