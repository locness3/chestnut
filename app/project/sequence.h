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
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QVector>
#include <memory>

#include "project/marker.h"
#include "project/selection.h"
#include "project/projectitem.h"
//FIXME: this is used EVERYWHERE. This has to be water-tight and heavily tested.

class Clip;
class Transition;
class Media;
typedef std::shared_ptr<Clip> ClipPtr;
typedef std::shared_ptr<Transition> TransitionPtr;

using SequencePtr = std::shared_ptr<Sequence>;
using SequenceUPtr = std::unique_ptr<Sequence>;
using SequenceWPtr = std::weak_ptr<Sequence>;

class Sequence : public project::ProjectItem {
public:

    Sequence() = default;
    Sequence(QVector<std::shared_ptr<Media>>& media_list, const QString& sequenceName);
    virtual ~Sequence() override;
    Sequence(const Sequence&& cpy) = delete;
    Sequence& operator=(const Sequence&) = delete;
    Sequence& operator=(const Sequence&&) = delete;

    Sequence(const Sequence& cpy);
    std::shared_ptr<Sequence> copy();
    void trackLimits(int& video_limit, int& audio_limit) const;
    long endFrame() const;
    void hardDeleteTransition(ClipPtr& c, const int type);

    bool setWidth(const int val);
    int width() const;
    bool setHeight(const int val);
    int height() const;
    double frameRate() const;
    bool setFrameRate(const double frameRate);
    int audioFrequency() const;
    bool setAudioFrequency(const int frequency);
    /**
     * @brief audioLayout from ffmpeg libresample
     * @return AV_CH_LAYOUT_*
     */
    int audioLayout() const;
    /**
     * @brief setAudioLayout using ffmpeg libresample
     * @param layout AV_CH_LAYOUT_* value from libavutil/channel_layout.h
     */
    void setAudioLayout(const int layout);

    void closeActiveClips(const int depth=0);

    QVector<Selection> selections_;
    QVector<ClipPtr> clips_;
    int save_id_ = 0;
    bool using_workarea_ = false;
    bool enable_workarea_ = true;
    long workarea_in_ = 0;
    long workarea_out_ = 0;
    QVector<TransitionPtr> transitions_;
    QVector<Marker> markers_;
    long playhead_ = 0;
    bool wrapper_sequence_ = false;

private:
    int width_ = -1;
    int height_ = -1;
    double frame_rate_ = -0.0;
    int audio_frequency_ = -1;
    int audio_layout_ = -1;
};

namespace global {
    // static variable for the currently active sequence
    extern SequencePtr sequence;
}

#endif // SEQUENCE_H
