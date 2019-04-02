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
#include <QThread>

#include "project/effect.h"
#include "project/sequence.h"
#include "project/sequenceitem.h"
#include "project/footage.h"
#include "project/media.h"
#include "project/timelineinfo.h"


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

class Clip : public project::SequenceItem,  public std::enable_shared_from_this<Clip>, protected QThread
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

    bool isActive(const long playhead);
    /**
     * @brief Identify if the clip is being cached
     * @return true==caching used
     */
    bool usesCacher() const;
    /**
     * @brief open_worker
     * @return true==success
     */
    bool openWorker();
    /**
     * @brief Free resources made via libav
     */
    void closeWorker();
    /**
     * @brief Open clip and allocate necessary resources
     * @param open_multithreaded
     * @return true==success
     */
    bool open(const bool open_multithreaded);
    /**
     * @brief mediaOpen
     * @return  true==clip's media has been opened
     */
    bool mediaOpen() const;
    /**
     * @brief Close this clip and free up resources
     * @param wait  Wait on cache?
     */
    void close(const bool wait);
    /**
     * @brief Close this clip and free up resources whilst waiting
     */
    void closeWithWait();
    /**
     * @brief Cache the clip at a certain point
     * @param playhead
     * @param reset
     * @param scrubbing
     * @param nests
     * @return  true==cached
     */
    bool cache(const long playhead, const bool do_reset, const bool scrubbing, QVector<ClipPtr>& nests);


    void resetAudio();
    void reset();
    void refresh();
    long clipInWithTransition();
    long timelineInWithTransition();
    long timelineOutWithTransition();
    long length();
    double mediaFrameRate();
    long maximumLength();
    void recalculateMaxLength();
    int width();
    int height();
    void refactorFrameRate(ComboAction* ca, double multiplier, bool change_timeline_points);

    // queue functions
    void clearQueue();
    void removeEarliestFromQueue();


    TransitionPtr openingTransition();
    TransitionPtr closingTransition();

    /**
     * @brief set frame cache to a position
     * @param playhead
     */
    void frame(const long playhead, bool& texture_failed);
    /**
     * @brief get_timecode
     * @param playhead
     * @return
     */
    double timecode(const long playhead);
    /**
     * @brief Identify if this clip is selected in the project's current sequence
     * @param containing
     * @return true==is selected
     */
    bool isSelected(const bool containing);

    //FIXME: all the class members
    SequencePtr sequence;

    // timeline variables (should be copied in copy())
    project::TimelineInfo timeline_info;

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
    int load_id{};
    bool undeletable;
    bool reached_end{};
    bool is_open{};
    bool replaced;
    std::atomic_bool ignore_reverse{false};
    int pix_fmt{};

    // caching functions
    bool use_existing_frame;
    bool multithreaded{};
    QWaitCondition can_cache;
    int max_queue_size{};
    QVector<AVFrame*> queue;
    QMutex queue_lock;
    QMutex lock;
    QMutex open_lock;
    int64_t last_invalid_ts{};

    // converters/filters
    AVFilterGraph* filter_graph;
    AVFilterContext* buffersink_ctx{};
    AVFilterContext* buffersrc_ctx{};

    // video playback variables
    QOpenGLFramebufferObject** fbo;
    std::unique_ptr<QOpenGLTexture> texture = nullptr;
    long texture_frame{};

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
    friend class ClipTest;
    struct {
        bool caching = false;
        // must be set before caching
        long playhead = 0;
        bool reset = false;
        bool scrubbing = false;
        bool interrupt = false;
        QVector<ClipPtr> nests = QVector<ClipPtr>();
    } cache_info;


    std::atomic_bool finished_opening{false};
    bool pkt_written{};

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
