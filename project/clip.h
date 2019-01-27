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
#ifndef CLIP_H
#define CLIP_H

#include <QWaitCondition>
#include <QMutex>
#include <QVector>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <memory>
#include <QMetaType>

#include "project/effect.h"
#include "project/sequence.h"
#include "project/sequenceitem.h"
#include "project/footage.h"
#include "project/media.h"

#define SKIP_TYPE_DISCARD 0
#define SKIP_TYPE_SEEK 1

class Transition;
class ComboAction;

struct AVFormatContext;
struct AVStream;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;
struct AVFilterGraph;
struct AVFilterContext;
struct AVDictionary;


using ClipPtr = std::shared_ptr<Clip>;
using ClipUPtr = std::unique_ptr<Clip>;
using ClipWPtr = std::weak_ptr<Clip>;

class Clip : public project::SequenceItem, QThread, std::enable_shared_from_this<Clip>
{
public:

    explicit Clip(SequencePtr s);
    virtual ~Clip() override;
    ClipPtr copy(SequencePtr s);

    Clip() = delete;
    Clip(const Clip&) = delete;
    Clip(const Clip&&) = delete;
    Clip& operator=(const Clip&) = delete;
    Clip& operator=(const Clip&&) = delete;

    virtual project::SequenceItemType getType() const;

    bool isActive(const long playhead);
    /**
     * @brief Identify if the clip is being cached
     * @return true==caching used
     */
    bool uses_cacher() const;
    /**
     * @brief open_worker
     * @return true==success
     */
    bool open_worker();
    /**
     * @brief Free resources made via libav
     */
    void close_worker();
    /**
     * @brief Open clip and allocate necessary resources
     * @param open_multithreaded
     * @return true==success
     */
    bool open(const bool open_multithreaded);
    /**
     * @brief Close this clip and free up resources
     * @param wait  Wait on cache?
     */
    void close(const bool wait);
    /**
     * @brief Close this clip and free up resources whilst waiting
     */
    void close_with_wait();
    /**
     * @brief Cache the clip at a certain point
     * @param playhead
     * @param reset
     * @param scrubbing
     * @param nests
     * @return  true==cached
     */
    bool cache(const long playhead, const bool do_reset, const bool scrubbing, QVector<ClipPtr>& nests);


    void reset_audio();
    void reset();
    void refresh();
    long get_clip_in_with_transition();
    long get_timeline_in_with_transition();
    long get_timeline_out_with_transition();
    long getLength();
    double getMediaFrameRate();
    long getMaximumLength();
    void recalculateMaxLength();
    int getWidth();
    int getHeight();
    void refactor_frame_rate(ComboAction* ca, double multiplier, bool change_timeline_points);

    // queue functions
    void queue_clear();
    void queue_remove_earliest();


    TransitionPtr get_opening_transition();
    TransitionPtr get_closing_transition();

    /**
     * @brief get_frame
     * @param playhead
     */
    void get_frame(const long playhead, bool& texture_failed);
    /**
     * @brief get_timecode
     * @param playhead
     * @return
     */
    double get_timecode(const long playhead);
    /**
     * @brief Identify if this clip is selected in the project's current sequence
     * @param containing
     * @return true==is selected
     */
    bool is_selected(const bool containing);

    //FIXME: all the class members
    SequencePtr sequence;
    // timeline variables (should be copied in copy())

    struct {
        bool enabled = true;
        long clip_in = 0;
        long in = 0;
        long out = 0;
        int track = 0;
        QString name = "";
        QColor color = {0,0,0};
        MediaPtr media = nullptr;
        int media_stream = -1;
        double speed = 1.0;
        double cached_fr = 0.0;
        bool reverse = false;
        bool maintain_audio_pitch = true;
        bool autoscale = true;
    } timeline_info;

    // other variables (should be deep copied/duplicated in copy())
    QList<EffectPtr> effects;
    QVector<int> linked;
    int opening_transition;
    int closing_transition;

    // media handling
    struct {
        AVFormatContext* formatCtx = nullptr;
        AVStream* stream = nullptr;
        AVCodec* codec = nullptr;
        AVCodecContext* codecCtx = nullptr;
        AVPacket* pkt = nullptr;
        AVFrame* frame = nullptr;
        AVDictionary* opts = nullptr;
        long calculated_length = -1;
    } media_handling;

    // temporary variables
    int load_id;
    bool undeletable;
    bool reached_end;
    bool pkt_written;
    bool is_open;
    bool finished_opening;
    bool replaced;
    bool ignore_reverse;
    int pix_fmt;

    // caching functions
    bool use_existing_frame;
    bool multithreaded;
    QWaitCondition can_cache;
    int max_queue_size;
    QVector<AVFrame*> queue;
    QMutex queue_lock;
    QMutex lock;
    QMutex open_lock;
    int64_t last_invalid_ts;

    // converters/filters
    AVFilterGraph* filter_graph;
    AVFilterContext* buffersink_ctx;
    AVFilterContext* buffersrc_ctx;

    // video playback variables
    QOpenGLFramebufferObject** fbo;
    std::unique_ptr<QOpenGLTexture> texture = nullptr;
    long texture_frame;

    struct AudioPlaybackInfo {
        // audio playback variables
        int64_t reverse_target = 0;
        int frame_sample_index = -1;
        int buffer_write = -1;
        bool reset = false;
        bool just_reset = false;
        long target_frame = -1;
    } audio_playback;


protected:
    virtual void run() override;

private:

    struct {
        bool caching = false;
        // must be set before caching
        long playhead = 0;
        bool reset = false;
        bool scrubbing = false;
        bool interrupt = false;
        QVector<ClipPtr> nests = QVector<ClipPtr>();
    } cache_info;

    void apply_audio_effects(const double timecode_start, AVFrame* frame, const int nb_bytes, QVector<ClipPtr>& nests);

    long playhead_to_frame(const long playhead);
    int64_t playhead_to_timestamp(const long playhead);
    bool retrieve_next_frame(AVFrame* frame);
    double playhead_to_seconds(const long playhead);
    int64_t seconds_to_timestamp(const double seconds);

    void cache_audio_worker(const bool scrubbing, QVector<ClipPtr>& nests);
    void cache_video_worker(const long playhead);


    /**
     * @brief To set up the caching thread?
     * @param playhead
     * @param reset
     * @param scrubbing
     * @param nests
     */
    void cache_worker(const long playhead, const bool reset, const bool scrubbing, QVector<ClipPtr>& nests);
    /**
     * @brief reset_cache
     * @param target_frame
     */
    void reset_cache(const long target_frame);

    // Comboaction::move_clip() or Clip::move()?
    void move(ComboAction& ca, const long iin, const long iout,
              const long iclip_in, const int itrack, const bool verify_transitions = true,
              const bool relative = false);


};

#endif // CLIP_H
