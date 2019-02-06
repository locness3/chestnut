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
const auto      RECURSION_LIMIT = 100;
const int       MAXIMUM_WIDTH              = 4096;
const int       MAXIMUM_HEIGHT             = 2160;
const int       MAXIMUM_FREQUENCY          = 192000;
const int       DEFAULT_WIDTH              = 1920;
const int       DEFAULT_HEIGHT             = 1080;
const double    DEFAULT_FRAMERATE          = 29.97;
const int       DEFAULT_AUDIO_FREQUENCY    = 48000;
const int       DEFAULT_LAYOUT             = 3; //TODO: what does this value mean?
}



Sequence::Sequence(QVector<std::shared_ptr<Media>>& media_list, const QString& sequenceName)
    : ProjectItem(sequenceName),
      width(DEFAULT_WIDTH),
      height(DEFAULT_HEIGHT),
      frame_rate(DEFAULT_FRAMERATE),
      audio_frequency(DEFAULT_AUDIO_FREQUENCY),
      audio_layout(DEFAULT_LAYOUT)
{
    bool got_video_values = false;
    bool got_audio_values = false;
    for (auto mda : media_list){
        if (mda == nullptr) {
            qCritical() << "Null MediaPtr";
            continue;
        }
        switch (mda->get_type()) {
        case MediaType::FOOTAGE:
        {
            auto ftg = mda->get_object<Footage>();
            if (!ftg->ready || got_video_values) {
                break;
            }
            for (const auto& ms : ftg->video_tracks) {
                width = ms.video_width;
                height = ms.video_height;
                if (!qFuzzyCompare(ms.video_frame_rate, 0.0)) {
                    frame_rate = ms.video_frame_rate * ftg->speed;

                    if (ms.video_interlacing != VIDEO_PROGRESSIVE) {
                        frame_rate *= 2;
                    }

                    // only break with a decent frame rate, otherwise there may be a better candidate
                    got_video_values = true;
                    break;
                }
            }//for
            if (!got_audio_values && !ftg->audio_tracks.empty()) {
                const auto& ms = ftg->audio_tracks.front();
                audio_frequency = ms.audio_frequency;
                got_audio_values = true;
            }
        }
            break;
        case MediaType::SEQUENCE:
        {
            if (auto seq = mda->get_object<Sequence>()) {
                width = seq->getWidth();
                height  = seq->getHeight();
                frame_rate = seq->getFrameRate();
                audio_frequency = seq->getAudioFrequency();
                audio_layout = seq->getAudioLayout();

                got_video_values = true;
                got_audio_values = true;
            }
        }
            break;
        default:
            qWarning() << "Unknown media type" << static_cast<int>(mda->get_type());
        }//switch
        if (got_video_values && got_audio_values) break;
    } //for

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


bool Sequence::setWidth(const int val)
{
    if ( (val % 2  == 0) && (val > 0) && (val <= MAXIMUM_WIDTH) ) {
        width = val;
        return true;
    }
    return false;
}
int Sequence::getWidth() const {
    return width;
}

bool Sequence::setHeight(const int val)
{
    if ( (val % 2  == 0) && (val > 0) && (val <= MAXIMUM_HEIGHT) ) {
        height = val;
        return true;
    }
    return false;
}
int Sequence::getHeight() const {
    return height;
}

double Sequence::getFrameRate() const {
    return frame_rate;
}

bool Sequence::setFrameRate(const double frameRate)
{
    if (frameRate > 0.0) {
        frame_rate = frameRate;
        return true;
    }
    return false;
}


int Sequence::getAudioFrequency() const {
    return audio_frequency;
}
bool Sequence::setAudioFrequency(const int frequency)
{
    if ( (frequency >= 0) && (frequency <= MAXIMUM_FREQUENCY) ) {
        audio_frequency = frequency;
        return true;
    }
    return false;
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
