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
      width_(DEFAULT_WIDTH),
      height_(DEFAULT_HEIGHT),
      frame_rate_(DEFAULT_FRAMERATE),
      audio_frequency_(DEFAULT_AUDIO_FREQUENCY),
      audio_layout_(DEFAULT_LAYOUT)
{
    bool got_video_values = false;
    bool got_audio_values = false;
    for (auto mda : media_list){
        if (mda == nullptr) {
            qCritical() << "Null MediaPtr";
            continue;
        }
        switch (mda->type()) {
        case MediaType::FOOTAGE:
        {
            auto ftg = mda->object<Footage>();
            if (!ftg->ready || got_video_values) {
                break;
            }
            for (const auto ms : ftg->video_tracks) {
                width_ = ms->video_width;
                height_ = ms->video_height;
                if (!qFuzzyCompare(ms->video_frame_rate, 0.0)) {
                    frame_rate_ = ms->video_frame_rate * ftg->speed;

                    if (ms->video_interlacing != VIDEO_PROGRESSIVE) {
                        frame_rate_ *= 2;
                    }

                    // only break with a decent frame rate, otherwise there may be a better candidate
                    got_video_values = true;
                    break;
                }
            }//for
            if (!got_audio_values && !ftg->audio_tracks.empty()) {
                const auto ms = ftg->audio_tracks.front();
                audio_frequency_ = ms->audio_frequency;
                got_audio_values = true;
            }
        }
            break;
        case MediaType::SEQUENCE:
        {
            if (auto seq = mda->object<Sequence>()) {
                width_ = seq->width();
                height_  = seq->height();
                frame_rate_ = seq->frameRate();
                audio_frequency_ = seq->audioFrequency();
                audio_layout_ = seq->audioLayout();

                got_video_values = true;
                got_audio_values = true;
            }
        }
            break;
        default:
            qWarning() << "Unknown media type" << static_cast<int>(mda->type());
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
    selections_(cpy.selections_),
    clips_(cpy.clips_),
    save_id_(cpy.save_id_),
    using_workarea_(cpy.using_workarea_),
    enable_workarea_(cpy.enable_workarea_),
    workarea_in_(cpy.workarea_in_),
    workarea_out_(cpy.workarea_out_),
    transitions_(cpy.transitions_),
    markers_(cpy.markers_),
    playhead_(cpy.playhead_),
    wrapper_sequence_(cpy.wrapper_sequence_),
    width_(cpy.width_),
    height_(cpy.height_),
    frame_rate_(cpy.frame_rate_),
    audio_frequency_(cpy.audio_frequency_),
    audio_layout_(cpy.audio_layout_)
{

}

std::shared_ptr<Sequence> Sequence::copy() {
    auto sqn = std::make_shared<Sequence>();
    sqn->name_ = QCoreApplication::translate("Sequence", "%1 (copy)").arg(name_);
    sqn->width_ = width_;
    sqn->height_ = height_;
    sqn->frame_rate_ = frame_rate_;
    sqn->audio_frequency_ = audio_frequency_;
    sqn->audio_layout_ = audio_layout_;
    sqn->clips_.resize(clips_.size());

    for (auto i=0;i<clips_.size();i++) {
        auto c = clips_.at(i);
        if (c == nullptr) {
            sqn->clips_[i] = nullptr;
        } else {
            auto copy(c->copy(sqn));
            copy->linked = c->linked;
            sqn->clips_[i] = copy;
        }
    }
    return sqn;
}

long Sequence::endFrame() const{
    auto end = 0L;
    for (auto clp : clips_) {
        if (clp && (clp->timeline_info.out > end) ) {
            end = clp->timeline_info.out;
        }
    }
    return end;
}

void Sequence::hardDeleteTransition(ClipPtr& c, const int type) {
    auto transition_index = (type == TA_OPENING_TRANSITION) ? c->opening_transition : c->closing_transition;
    if (transition_index > -1) {
        auto del = true;

        auto transition_for_delete = transitions_.at(transition_index);
        if (!transition_for_delete->secondary_clip.expired()) {
            for (auto comp : clips_) {
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
            transitions_[transition_index] = nullptr;
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
        width_ = val;
        return true;
    }
    return false;
}
int Sequence::width() const {
    return width_;
}

bool Sequence::setHeight(const int val)
{
    if ( (val % 2  == 0) && (val > 0) && (val <= MAXIMUM_HEIGHT) ) {
        height_ = val;
        return true;
    }
    return false;
}
int Sequence::height() const {
    return height_;
}

double Sequence::frameRate() const {
    return frame_rate_;
}

bool Sequence::setFrameRate(const double frameRate)
{
    if (frameRate > 0.0) {
        frame_rate_ = frameRate;
        return true;
    }
    return false;
}


int Sequence::audioFrequency() const {
    return audio_frequency_;
}
bool Sequence::setAudioFrequency(const int frequency)
{
    if ( (frequency >= 0) && (frequency <= MAXIMUM_FREQUENCY) ) {
        audio_frequency_ = frequency;
        return true;
    }
    return false;
}

/**
 * @brief getAudioLayout from ffmpeg libresample
 * @return AV_CH_LAYOUT_*
 */
int Sequence::audioLayout() const {
    return audio_layout_;
}
/**
 * @brief setAudioLayout using ffmpeg libresample
 * @param layout AV_CH_LAYOUT_* value from libavutil/channel_layout.h
 */
void Sequence::setAudioLayout(const int layout) {
    audio_layout_ = layout;
}


void Sequence::closeActiveClips(const int depth)
{
    if (depth > RECURSION_LIMIT) return;
    for (auto c : clips_) {
        if (!c) {
            qWarning() << "Null Clip ptr";
            continue;
        }
        if (c->timeline_info.media && (c->timeline_info.media->type() == MediaType::SEQUENCE) ) {
            if (auto seq = c->timeline_info.media->object<Sequence>()) {
                seq->closeActiveClips(depth + 1);
            }
        }
        c->close(true);
    }//for
}

void Sequence::trackLimits(int& video_limit, int& audio_limit) const {
    video_limit = 0;
    audio_limit = 0;
    for (auto clp : clips_) {
        if (clp == nullptr) {
            continue;
        }
        if ( (clp->timeline_info.isVideo()) && (clp->timeline_info.track < video_limit) ) { // video clip
            video_limit = clp->timeline_info.track;
        } else if (clp->timeline_info.track > audio_limit) {
            audio_limit = clp->timeline_info.track;
        }
    }//for
}

// static variable for the currently active sequence
SequencePtr global::sequence;
