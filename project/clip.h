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

#define SKIP_TYPE_DISCARD 0
#define SKIP_TYPE_SEEK 1

class Cacher;
class Transition;
class ComboAction;
class Media;


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

class Clip : public project::SequenceItem
{
public:
    explicit Clip(SequencePtr s);
    virtual ~Clip();
    /**
     * @brief Copy constructor
     * @param cpy
     */
    Clip(const Clip& cpy);

    ClipPtr copy(SequencePtr s);

    virtual project::SequenceItemType_E getType() const;

    bool isActive(const long playhead);
    /**
     * @brief Identify if the clip is being cached
     * @return true==caching used
     */
    bool uses_cacher() const;
    /**
     * @brief Free resources made via libav
     */
    void close_worker();
    /**
     * @brief Close this clip and free up resources
     * @param wait  Wait on cache?
     */
    void close(const bool wait);

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


    Transition* get_opening_transition();
    Transition* get_closing_transition();

    //FIXME: all the class members
    SequencePtr sequence;
    // timeline variables (should be copied in copy())

    struct {
        bool enabled = true;
        long clip_in = 0;
        long in = 0;
        long out = 0;
        int track = 0;
        QString name;
        QColor color;
        Media* media = NULL;
        int media_stream;
        double speed = 1.0;
        double cached_fr;
        bool reverse = false;
        bool maintain_audio_pitch;
        bool autoscale;
    } timeline_info;

	// other variables (should be deep copied/duplicated in copy())
    QList<EffectPtr> effects;
    QVector<int> linked;
    int opening_transition;
    int closing_transition;

	// media handling
    struct {
        AVFormatContext* formatCtx = NULL;
        AVStream* stream = NULL;
        AVCodec* codec = NULL;
        AVCodecContext* codecCtx = NULL;
        AVPacket* pkt = NULL;
        AVFrame* frame = NULL;
        AVDictionary* opts = NULL;
        long calculated_length = -1;
    } media_handling;

	// temporary variables
    int load_id;
    bool undeletable;
	bool reached_end;
	bool pkt_written;
    bool open;
    bool finished_opening;
    bool replaced;
	bool ignore_reverse;
	int pix_fmt;

	// caching functions
	bool use_existing_frame;
    bool multithreaded;
	Cacher* cacher;
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
    QOpenGLTexture* texture;
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
private:

    // Comboaction::move_clip() or Clip::move()?
    void move(ComboAction& ca, const long iin, const long iout,
              const long iclip_in, const int itrack, const bool verify_transitions = true,
              const bool relative = false);


    // Explicitly impl as required
    Clip();
    const Clip& operator=(const Clip& rhs);
};

typedef std::shared_ptr<Clip> ClipPtr;

#endif // CLIP_H
