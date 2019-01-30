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

namespace {
    const auto RECURSION_LIMIT = 100;
}

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
    ProjectItem(),
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
    auto sqn = std::make_shared<Sequence>();
    sqn->name = QCoreApplication::translate("Sequence", "%1 (copy)").arg(name);
    sqn->width = width;
    sqn->height = height;
    sqn->frame_rate = frame_rate;
    sqn->audio_frequency = audio_frequency;
    sqn->audio_layout = audio_layout;
    sqn->clips.resize(clips.size());

    for (auto i=0;i<clips.size();i++) {
        auto c = clips.at(i);
        if (c == nullptr) {
            sqn->clips[i] = nullptr;
        } else {
            auto copy(c->copy(sqn));
            copy->linked = c->linked;
            sqn->clips[i] = copy;
        }
    }
    return sqn;
}

long Sequence::getEndFrame() const{
    auto end = 0L;
    for (auto clp : clips) {
        if (clp && (clp->timeline_info.out > end) ) {
            end = clp->timeline_info.out;
        }
    }
    return end;
}

void Sequence::hard_delete_transition(ClipPtr& c, const int type) {
    auto transition_index = (type == TA_OPENING_TRANSITION) ? c->opening_transition : c->closing_transition;
    if (transition_index > -1) {
        auto del = true;

        auto transition_for_delete = transitions.at(transition_index);
        if (!transition_for_delete->secondary_clip.expired()) {
            for (auto comp : clips) {
                if (!comp){
                    continue;
                }
                if (c != comp){ //TODO: check this is the correct comparison
                    continue;
                }
                if ( (c->opening_transition == transition_index) || (c->closing_transition == transition_index) ) {
                    if (type == TA_OPENING_TRANSITION) {
                        // convert to closing transition
                        transition_for_delete->parent_clip = transition_for_delete->secondary_clip.lock();
                    }

                    del = false;
                    transition_for_delete->secondary_clip.reset();
                }
            }//for
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


void Sequence::closeActiveClips(const int depth)
{
    if (depth > RECURSION_LIMIT) return;
    for (auto c : clips) {
        if (!c) {
            qWarning() << "Null Clip ptr";
            continue;
        }
        if (c->timeline_info.media && (c->timeline_info.media->get_type() == MediaType::SEQUENCE) ) {
            if (auto seq = c->timeline_info.media->get_object<Sequence>()) {
                seq->closeActiveClips(depth + 1);
            }
        }
        c->close(true);
    }//for
}

void Sequence::getTrackLimits(int& video_limit, int& audio_limit) const {
    video_limit = 0;
    audio_limit = 0;
    for (auto clp : clips) {
        if (clp == nullptr) {
            continue;
        }
        if ( (clp->timeline_info.track < 0) && (clp->timeline_info.track < video_limit) ) { // video clip
            video_limit = clp->timeline_info.track;
        } else if (clp->timeline_info.track > audio_limit) {
            audio_limit = clp->timeline_info.track;
        }
    }//for
}

// static variable for the currently active sequence
SequencePtr global::sequence;
