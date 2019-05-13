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
#include <QMap>
#include <memory>

#include "project/marker.h"
#include "project/selection.h"
#include "project/projectitem.h"

//FIXME: this is used EVERYWHERE. This has to be water-tight and heavily tested.

class Clip;
class Media;
class Transition;

using ClipPtr = std::shared_ptr<Clip>;
using TransitionPtr = std::shared_ptr<Transition>;
using TransitionWPtr = std::weak_ptr<Transition>;

using SequencePtr = std::shared_ptr<Sequence>;
using SequenceUPtr = std::unique_ptr<Sequence>;
using SequenceWPtr = std::weak_ptr<Sequence>;


Q_DECLARE_METATYPE(SequencePtr)


struct Track {
    bool enabled{true};
    bool locked{false};
};

class Sequence : public std::enable_shared_from_this<Sequence>, public project::ProjectItem {
public:

    Sequence() = default;
    explicit Sequence(const std::shared_ptr<Media>& parent);
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

    /**
     * @brief Obtain populated tracks at a position in the sequence
     * @param frame   Position
     * @return        Set of tracks
     */
    QSet<int> tracks(const long frame) const;

    /**
     * @brief         Obtain the populated track count in the sequence
     * @param video   true==video tracks, false==audio tracks
     * @return        track count
     */
    int trackCount(const bool video) const;
    /**
     * @brief               Identify if the track is enabled (for viewing and rendering)
     * @param track_number
     * @return              true==track is enabled
     */
    bool trackEnabled(const int track_number) const;
    /**
     * @brief               Identify if the track is locked editing in the timeline
     * @param track_number
     * @return              true==track is locked
     */
    bool trackLocked(const int track_number) const;

    /**
     * @brief         Obtain all clips at a position in the sequence
     * @param frame   Position
     * @return        list of clips
     */
    QVector<ClipPtr> clips(const long frame) const;

    void closeActiveClips(const int32_t depth=0);
    /**
     * @brief     Obtain a clip from this sequence
     * @param id  Clip id
     * @return    Clip instance or null
     */
    ClipPtr clip(const int32_t id);
    /**
     * @brief     Remove a clip from this sequence
     * @param id  Clip id
     */
    void deleteClip(const int32_t id);

    virtual bool load(QXmlStreamReader& stream) override;
    virtual bool save(QXmlStreamWriter& stream) const override;

    /**
     * @brief         Lock a track to prevent editing (via the timeline) of contained clips until unlocked
     * @param number  The track number
     * @param lock    true==lock
     */
    void lockTrack(const int number, const bool lock);
    /**
     * @brief         Enable/disable a track (for the timeline and sequence viewer)
     * @param number  The track number
     * @param enable  true==enable
     */
    void enableTrack(const int number, const bool enable);

    QVector<Selection> selections_;
    QVector<ClipPtr> clips_;
    int32_t save_id_ = 0; //FIXME: fudge
    struct {
        bool using_ = false;
        bool enabled_ = true;
        int64_t in_ = 0;
        int64_t out_ = 0;
    } workarea_;

    QVector<TransitionPtr> transitions_;
    int64_t playhead_ = 0;
    bool wrapper_sequence_ = false;
    std::weak_ptr<Media> parent_mda{};

private:
    friend class SequenceTest;
    int32_t width_ = -1;
    int32_t height_ = -1;
    double frame_rate_ = -0.0;
    int32_t audio_frequency_ = -1;
    int32_t audio_layout_ = -1;
    QMap<int, Track> tracks_;

    bool loadWorkArea(QXmlStreamReader& stream);
};

namespace global {
    // static variable for the currently active sequence
    extern SequencePtr sequence;
}

#endif // SEQUENCE_H
