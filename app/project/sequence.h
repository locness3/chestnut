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

class Sequence : public std::enable_shared_from_this<Sequence>, public project::ProjectItem, public project::IXMLStreamer {
public:

    Sequence() = default;
    Sequence(QVector<std::shared_ptr<Media>>& media_list, const QString& sequenceName);

    std::shared_ptr<Sequence> copy();
    std::pair<int64_t,int64_t> trackLimits() const;
    int64_t endFrame() const;
    void hardDeleteTransition(const ClipPtr& c, const int32_t type);

    bool setWidth(const int32_t val);
    int32_t width() const;
    bool setHeight(const int32_t val);
    int32_t height() const;
    double frameRate() const;
    bool setFrameRate(const double frameRate);
    int32_t audioFrequency() const;
    bool setAudioFrequency(const int32_t frequency);
    /**
     * @brief audioLayout from ffmpeg libresample
     * @return AV_CH_LAYOUT_*
     */
    int32_t audioLayout() const;
    /**
     * @brief setAudioLayout using ffmpeg libresample
     * @param layout AV_CH_LAYOUT_* value from libavutil/channel_layout.h
     */
    void setAudioLayout(const int32_t layout);

    void closeActiveClips(const int32_t depth=0);

    virtual bool load(QXmlStreamReader& stream) override;
    virtual bool save(QXmlStreamWriter& stream) const override;

    QVector<Selection> selections_;
    QVector<ClipPtr> clips_;
    int32_t save_id_ = 0;
    struct {
        bool using_ = false;
        bool enabled_ = true;
        int64_t in_ = 0;
        int64_t out_ = 0;
    } workarea_;

    QVector<TransitionPtr> transitions_;
    int64_t playhead_ = 0;
    bool wrapper_sequence_ = false;

private:
    friend class SequenceTest;
    int32_t width_ = -1;
    int32_t height_ = -1;
    double frame_rate_ = -0.0;
    int32_t audio_frequency_ = -1;
    int32_t audio_layout_ = -1;

    bool loadWorkArea(QXmlStreamReader& stream);
};

namespace global {
    // static variable for the currently active sequence
    extern SequencePtr sequence;
}

#endif // SEQUENCE_H
