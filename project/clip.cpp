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
#include "clip.h"

#include <QtMath>

#include "project/effect.h"
#include "project/transition.h"
#include "project/footage.h"
#include "io/config.h"
#include "playback/playback.h"
#include "playback/audio.h"
#include "panels/project.h"
#include "project/sequence.h"
#include "panels/timeline.h"
#include "panels/panels.h"
#include "panels/viewer.h"
#include "project/media.h"
#include "io/clipboard.h"
#include "io/avtogl.h"
#include "undo.h"
#include "debug.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

namespace {
    const bool WAIT_ON_CLOSE = true;
    const AVSampleFormat SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;
    const int AUDIO_SAMPLES = 2048;
    const int AUDIO_BUFFER_PADDING = 2048;
}

double bytes_to_seconds(const int nb_bytes, const int nb_channels, const int sample_rate) {
    return (static_cast<double>(nb_bytes >> 1) / nb_channels / sample_rate);
}

Clip::Clip(SequencePtr s) :
    sequence(s),
    opening_transition(-1),
    closing_transition(-1),
    undeletable(false),
    replaced(false),
    ignore_reverse(false),
    use_existing_frame(false),
    filter_graph(NULL),
    fbo(NULL),
    media_handling()
{
    media_handling.pkt = av_packet_alloc();
    timeline_info.autoscale = e_config.autoscale_by_default;
    reset();
}


Clip::~Clip() {
    if (is_open) {
        close(WAIT_ON_CLOSE);
    }

    //FIXME:
    //    if (opening_transition != -1) this->sequence->hard_delete_transition(this, TA_OPENING_TRANSITION);
    //    if (closing_transition != -1) this->sequence->hard_delete_transition(this, TA_CLOSING_TRANSITION);

    effects.clear();
    av_packet_free(&media_handling.pkt);
}

/**
 * @brief Copy constructor
 * @param cpy
 */
Clip::Clip(const Clip& cpy)
{

}

ClipPtr Clip::copy(SequencePtr s) {
    ClipPtr copyClip(new Clip(s));

    copyClip->timeline_info.enabled = timeline_info.enabled;
    copyClip->timeline_info.name = timeline_info.name;
    copyClip->timeline_info.clip_in = timeline_info.clip_in;
    copyClip->timeline_info.in = timeline_info.in;
    copyClip->timeline_info.out = timeline_info.out;
    copyClip->timeline_info.track = timeline_info.track;
    copyClip->timeline_info.color = timeline_info.color;
    copyClip->timeline_info.media = timeline_info.media;
    copyClip->timeline_info.media_stream = timeline_info.media_stream;
    copyClip->timeline_info.autoscale = timeline_info.autoscale;
    copyClip->timeline_info.speed = timeline_info.speed;
    copyClip->timeline_info.maintain_audio_pitch = timeline_info.maintain_audio_pitch;
    copyClip->timeline_info.reverse = timeline_info.reverse;

    for (int i=0; i<effects.size(); i++) {
         //TODO:hmm
        copyClip->effects.append(effects.at(i)->copy(copyClip));
    }

    copyClip->timeline_info.cached_fr = (this->sequence == NULL) ? timeline_info.cached_fr : this->sequence->getFrameRate();

    if (get_opening_transition() != NULL && get_opening_transition()->secondary_clip == NULL) {
        copyClip->opening_transition = get_opening_transition()->copy(copyClip, NULL);
    }
    if (get_closing_transition() != NULL && get_closing_transition()->secondary_clip == NULL) {
        copyClip->closing_transition = get_closing_transition()->copy(copyClip, NULL);
    }
    copyClip->recalculateMaxLength();

    return copyClip;
}


project::SequenceItemType_E Clip::getType() const {
    return project::SEQUENCE_ITEM_CLIP;
}


bool Clip::isActive(const long playhead) {
    if (timeline_info.enabled) {
        if (sequence != NULL) {
            if ( get_timeline_in_with_transition() < (playhead + (ceil(sequence->getFrameRate()*2)) ) )  {                      //TODO:what are we checking?
                if (get_timeline_out_with_transition() > playhead) {                                                            //TODO:what are we checking?
                    if ( (playhead - get_timeline_in_with_transition() + get_clip_in_with_transition()) < getMaximumLength() ){ //TODO:what are we checking?
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/**
 * @brief Identify if the clip is being cached
 * @return true==caching used
 */
bool Clip::uses_cacher() const {
    if (( (timeline_info.media == NULL) && (timeline_info.track >= 0) )
            || ( (timeline_info.media != NULL) && (timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE))) {
        return true;
    }
    return false;
}

/**
 * @brief open_worker
 * @return true==success
 */
bool Clip::open_worker() {
    if (timeline_info.media == NULL) {
        if (timeline_info.track >= 0) {
            media_handling.frame = av_frame_alloc();
            media_handling.frame->format = SAMPLE_FORMAT;
            media_handling.frame->channel_layout = sequence->getAudioLayout();
            media_handling.frame->channels = av_get_channel_layout_nb_channels(media_handling.frame->channel_layout);
            media_handling.frame->sample_rate = current_audio_freq();
            media_handling.frame->nb_samples = AUDIO_SAMPLES;
            av_frame_make_writable(media_handling.frame);
            if (av_frame_get_buffer(media_handling.frame, 0)) {
                dout << "[ERROR] Could not allocate buffer for tone clip";
            }
            audio_playback.reset = true;
        }
    } else if (timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
        // opens file resource for FFmpeg and prepares Clip struct for playback
        FootagePtr m = timeline_info.media->get_object<Footage>();
        QByteArray ba = m->url.toUtf8();
        const char* filename = ba.constData();
        const FootageStream* ms = m->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream);

        int errCode = avformat_open_input(
                    &media_handling.formatCtx,
                    filename,
                    NULL,
                    NULL
                    );
        if (errCode != 0) {
            char err[1024];
            av_strerror(errCode, err, 1024);
            dout << "[ERROR] Could not open" << filename << "-" << err;
            return false;
        }

        errCode = avformat_find_stream_info(media_handling.formatCtx, NULL);
        if (errCode < 0) {
            char err[1024];
            av_strerror(errCode, err, 1024);
            dout << "[ERROR] Could not open" << filename << "-" << err;
            return false;
        }

        av_dump_format(media_handling.formatCtx, 0, filename, 0);

        media_handling.stream = media_handling.formatCtx->streams[ms->file_index];
        media_handling.codec = avcodec_find_decoder(media_handling.stream->codecpar->codec_id);
        media_handling.codecCtx = avcodec_alloc_context3(media_handling.codec);
        avcodec_parameters_to_context(media_handling.codecCtx, media_handling.stream->codecpar);

        if (ms->infinite_length) {
            max_queue_size = 1;
        } else {
            max_queue_size = 0;
            if (e_config.upcoming_queue_type == FRAME_QUEUE_TYPE_FRAMES) {
                max_queue_size += qCeil(e_config.upcoming_queue_size);
            } else {
                max_queue_size += qCeil(ms->video_frame_rate * m->speed * e_config.upcoming_queue_size);
            }
            if (e_config.previous_queue_type == FRAME_QUEUE_TYPE_FRAMES) {
                max_queue_size += qCeil(e_config.previous_queue_size);
            } else {
                max_queue_size += qCeil(ms->video_frame_rate * m->speed * e_config.previous_queue_size);
            }
        }

        if (ms->video_interlacing != VIDEO_PROGRESSIVE) max_queue_size *= 2;

        media_handling.opts = NULL;

        // optimized decoding settings
        if ((media_handling.stream->codecpar->codec_id != AV_CODEC_ID_PNG &&
             media_handling.stream->codecpar->codec_id != AV_CODEC_ID_APNG &&
             media_handling.stream->codecpar->codec_id != AV_CODEC_ID_TIFF &&
             media_handling.stream->codecpar->codec_id != AV_CODEC_ID_PSD)
                || !e_config.disable_multithreading_for_images) {
            av_dict_set(&media_handling.opts, "threads", "auto", 0);
        }
        if (media_handling.stream->codecpar->codec_id == AV_CODEC_ID_H264) {
            av_dict_set(&media_handling.opts, "tune", "fastdecode", 0);
            av_dict_set(&media_handling.opts, "tune", "zerolatency", 0);
        }

        // Open codec
        if (avcodec_open2(media_handling.codecCtx, media_handling.codec, &media_handling.opts) < 0) {
            dout << "[ERROR] Could not open codec";
        }

        // allocate filtergraph
        filter_graph = avfilter_graph_alloc();
        if (filter_graph == NULL) {
            dout << "[ERROR] Could not create filtergraph";
        }
        char filter_args[512];

        if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            snprintf(filter_args, sizeof(filter_args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                     media_handling.stream->codecpar->width,
                     media_handling.stream->codecpar->height,
                     media_handling.stream->codecpar->format,
                     media_handling.stream->time_base.num,
                     media_handling.stream->time_base.den,
                     media_handling.stream->codecpar->sample_aspect_ratio.num,
                     media_handling.stream->codecpar->sample_aspect_ratio.den
                     );

            avfilter_graph_create_filter(&buffersrc_ctx, avfilter_get_by_name("buffer"), "in", filter_args, NULL, filter_graph);
            avfilter_graph_create_filter(&buffersink_ctx, avfilter_get_by_name("buffersink"), "out", NULL, NULL, filter_graph);

            AVFilterContext* last_filter = buffersrc_ctx;

            if (ms->video_interlacing != VIDEO_PROGRESSIVE) {
                AVFilterContext* yadif_filter;
                char yadif_args[100];
                snprintf(yadif_args, sizeof(yadif_args), "mode=3:parity=%d", ((ms->video_interlacing == VIDEO_TOP_FIELD_FIRST) ? 0 : 1)); // there's a CUDA version if we start using nvdec/nvenc
                avfilter_graph_create_filter(&yadif_filter, avfilter_get_by_name("yadif"), "yadif", yadif_args, NULL, filter_graph);

                avfilter_link(last_filter, 0, yadif_filter, 0);
                last_filter = yadif_filter;
            }

            /* stabilization code */
            bool stabilize = false;
            if (stabilize) {
                AVFilterContext* stab_filter;
                int stab_ret = avfilter_graph_create_filter(&stab_filter, avfilter_get_by_name("vidstabtransform"), "vidstab", "input=/media/matt/Home/samples/transforms.trf", NULL, filter_graph);
                if (stab_ret < 0) {
                    char err[100];
                    av_strerror(stab_ret, err, sizeof(err));
                } else {
                    avfilter_link(last_filter, 0, stab_filter, 0);
                    last_filter = stab_filter;
                }
            }

            enum AVPixelFormat valid_pix_fmts[] = {
                AV_PIX_FMT_RGB24,
                AV_PIX_FMT_RGBA,
                AV_PIX_FMT_NONE
            };

            pix_fmt = avcodec_find_best_pix_fmt_of_list(valid_pix_fmts, static_cast<enum AVPixelFormat>(media_handling.stream->codecpar->format), 1, NULL);
            const char* chosen_format = av_get_pix_fmt_name(static_cast<enum AVPixelFormat>(pix_fmt));
            char format_args[100];
            snprintf(format_args, sizeof(format_args), "pix_fmts=%s", chosen_format);

            AVFilterContext* format_conv;
            avfilter_graph_create_filter(&format_conv, avfilter_get_by_name("format"), "fmt", format_args, NULL, filter_graph);
            avfilter_link(last_filter, 0, format_conv, 0);

            avfilter_link(format_conv, 0, buffersink_ctx, 0);

            avfilter_graph_config(filter_graph, NULL);
        } else if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (media_handling.codecCtx->channel_layout == 0) media_handling.codecCtx->channel_layout = av_get_default_channel_layout(media_handling.stream->codecpar->channels);

            // set up cache
            queue.append(av_frame_alloc());
            if (timeline_info.reverse) {
                AVFrame* reverse_frame = av_frame_alloc();

                reverse_frame->format = SAMPLE_FORMAT;
                reverse_frame->nb_samples = current_audio_freq()*2;
                reverse_frame->channel_layout = sequence->getAudioLayout();
                reverse_frame->channels = av_get_channel_layout_nb_channels(sequence->getAudioLayout());
                av_frame_get_buffer(reverse_frame, 0);

                queue.append(reverse_frame);
            }

            snprintf(filter_args, sizeof(filter_args), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
                     media_handling.stream->time_base.num,
                     media_handling.stream->time_base.den,
                     media_handling.stream->codecpar->sample_rate,
                     av_get_sample_fmt_name(media_handling.codecCtx->sample_fmt),
                     media_handling.codecCtx->channel_layout
                     );

            avfilter_graph_create_filter(&buffersrc_ctx, avfilter_get_by_name("abuffer"), "in", filter_args, NULL, filter_graph);
            avfilter_graph_create_filter(&buffersink_ctx, avfilter_get_by_name("abuffersink"), "out", NULL, NULL, filter_graph);

            enum AVSampleFormat sample_fmts[] = { SAMPLE_FORMAT,  static_cast<AVSampleFormat>(-1) };
            if (av_opt_set_int_list(buffersink_ctx, "sample_fmts", sample_fmts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
                dout << "[ERROR] Could not set output sample format";
            }

            int64_t channel_layouts[] = { AV_CH_LAYOUT_STEREO, static_cast<AVSampleFormat>(-1) };
            if (av_opt_set_int_list(buffersink_ctx, "channel_layouts", channel_layouts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
                dout << "[ERROR] Could not set output sample format";
            }

            int target_sample_rate = current_audio_freq();

            double playback_speed = timeline_info.speed * m->speed;

            if (qFuzzyCompare(playback_speed, 1.0)) {
                avfilter_link(buffersrc_ctx, 0, buffersink_ctx, 0);
            } else if (timeline_info.maintain_audio_pitch) {
                AVFilterContext* previous_filter = buffersrc_ctx;
                AVFilterContext* last_filter = buffersrc_ctx;

                char speed_param[10];

                //				if (playback_speed != 1.0) {
                double base = (playback_speed > 1.0) ? 2.0 : 0.5;

                double speedlog = log(playback_speed) / log(base);
                int whole2 = qFloor(speedlog);
                speedlog -= whole2;

                if (whole2 > 0) {
                    snprintf(speed_param, sizeof(speed_param), "%f", base);
                    for (int i=0;i<whole2;i++) {
                        AVFilterContext* tempo_filter = NULL;
                        avfilter_graph_create_filter(&tempo_filter, avfilter_get_by_name("atempo"), "atempo", speed_param, NULL, filter_graph);
                        avfilter_link(previous_filter, 0, tempo_filter, 0);
                        previous_filter = tempo_filter;
                    }
                }

                snprintf(speed_param, sizeof(speed_param), "%f", qPow(base, speedlog));
                last_filter = NULL;
                avfilter_graph_create_filter(&last_filter, avfilter_get_by_name("atempo"), "atempo", speed_param, NULL, filter_graph);
                avfilter_link(previous_filter, 0, last_filter, 0);
                //				}

                avfilter_link(last_filter, 0, buffersink_ctx, 0);
            } else {
                target_sample_rate = qRound64(target_sample_rate / playback_speed);
                avfilter_link(buffersrc_ctx, 0, buffersink_ctx, 0);
            }

            int sample_rates[] = { target_sample_rate, 0 };
            if (av_opt_set_int_list(buffersink_ctx, "sample_rates", sample_rates, 0, AV_OPT_SEARCH_CHILDREN) < 0) {
                dout << "[ERROR] Could not set output sample rates";
            }

            avfilter_graph_config(filter_graph, NULL);

            audio_playback.reset = true;
        }

        media_handling.frame = av_frame_alloc();
    }

    for (int i=0;i<effects.size();i++) {
        effects.at(i)->open();
    }

    finished_opening = true;

    dinfo << "Clip opened on track" << timeline_info.track;
    return true;
}

/**
 * @brief Free resources made via libav
 */
void Clip::close_worker() {
    finished_opening = false;

    if (timeline_info.media != NULL && timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
        queue_clear();
        // clear resources allocated via libav
        avfilter_graph_free(&filter_graph);
        avcodec_close(media_handling.codecCtx);
        avcodec_free_context(&media_handling.codecCtx);
        av_dict_free(&media_handling.opts);
        avformat_close_input(&media_handling.formatCtx);
    }

    av_frame_free(&media_handling.frame);

    reset();

    dinfo << "Clip closed on track" << timeline_info.track;
}



/**
 * @brief Open clip and allocate necessary resources
 * @param open_multithreaded
 * @return true==success
 */
bool Clip::open(const bool open_multithreaded) {
    if (is_open) {
        return false;
    }
    if (uses_cacher()) {
        multithreaded = open_multithreaded;
        if (multithreaded) {
            if (open_lock.tryLock()) {
                // maybe keep cacher instance in memory while clip exists for performance?
//                cache_thread = new Cacher(std::shared_ptr<Clip>(this)); // TODO: check this is the right why
                QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
                this->start((timeline_info.track < 0) ? QThread::HighPriority : QThread::TimeCriticalPriority);
            }
        } else {
            finished_opening = false;
            is_open = true;
            open_worker();
        }
    } else {
        is_open = true;
    }
    return true;
}


/**
 * @brief Close this clip and free up resources
 * @param wait  Wait on cache?
 */
void Clip::close(const bool wait) {
    if (is_open) {
        // destroy opengl texture in main thread
        if (texture != NULL) {
            delete texture;
            texture = NULL;
        }

        foreach (EffectPtr eff, effects) {
            if (eff != NULL) {
                if (eff->is_open()) {
                    eff->close();
                }
            }
        }

        if (fbo != NULL) {
            delete fbo[0];
            delete fbo[1];
            delete [] fbo;
            fbo = NULL;
        }

        if (uses_cacher()) {
            if (multithreaded) {
                cache_info.caching = false;
                can_cache.wakeAll();
                if (wait) {
                    open_lock.lock();
                    open_lock.unlock();
                }
            } else {
                close_worker();
            }
        } else {
            if (timeline_info.media != NULL && (timeline_info.media->get_type() == MEDIA_TYPE_SEQUENCE)) {
                closeActiveClips(timeline_info.media->get_object<Sequence>());
            }

            is_open = false;
        }
    }
}


/**
 * @brief Cache the clip at a certian point
 * @param playhead
 * @param reset
 * @param scrubbing
 * @param nests
 * @return  true==cached
 */
bool Clip::cache(const long playhead, const bool do_reset, const bool scrubbing, QVector<ClipPtr>& nests) {
    if (!uses_cacher()) {
        dwarning << "Not using caching";
        return false;
    }

    if (multithreaded) {
        cache_info.playhead = playhead;
        cache_info.reset = do_reset;
        cache_info.nests = nests;
        cache_info.scrubbing = scrubbing;
        if (cache_info.reset && queue.size() > 0) {
            cache_info.interrupt = true;
        }

        can_cache.wakeAll();
    } else {
        cache_worker(playhead, do_reset, scrubbing, nests);
    }
    return true;
}



void Clip::reset() {
    audio_playback.just_reset = false;
    is_open = false;
    finished_opening = false;
    pkt_written = false;
    audio_playback.reset = false;
    audio_playback.frame_sample_index = -1;
    audio_playback.buffer_write = false;
    texture_frame = -1;
    last_invalid_ts = -1;
    //TODO: check memory has been freed correctly or needs to be done here
    media_handling.formatCtx = NULL;
    media_handling.stream = NULL;
    media_handling.codec = NULL;
    media_handling.codecCtx = NULL;
    texture = NULL;
}

void Clip::reset_audio() {
    if (timeline_info.media == NULL || timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
        audio_playback.reset = true;
        audio_playback.frame_sample_index = -1;
        audio_playback.buffer_write = 0;
    } else if (timeline_info.media->get_type() == MEDIA_TYPE_SEQUENCE) {
        SequencePtr nested_sequence = timeline_info.media->get_object<Sequence>();
        for (int i=0;i<nested_sequence->clips.size();i++) {
            ClipPtr c(nested_sequence->clips.at(i));
            if (c != NULL) reset_audio();
        }
    }
}

void Clip::refresh() {
    // validates media if it was replaced
    if (replaced && timeline_info.media != NULL && timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
        FootagePtr m = timeline_info.media->get_object<Footage>();

        if (timeline_info.track < 0 && m->video_tracks.size() > 0)  {
            timeline_info.media_stream = m->video_tracks.at(0).file_index;
        } else if (timeline_info.track >= 0 && m->audio_tracks.size() > 0) {
            timeline_info.media_stream = m->audio_tracks.at(0).file_index;
        }
    }
    replaced = false;

    // reinitializes all effects... just in case
    for (int i=0;i<effects.size();i++) {
        effects.at(i)->refresh();
    }

    recalculateMaxLength();
}

void Clip::queue_clear() {
    while (queue.size() > 0) {
        av_frame_free(&queue.first());
        queue.removeFirst();
    }
}

void Clip::queue_remove_earliest() {
    int earliest_frame = 0;
    for (int i=1;i<queue.size();i++) {
        // TODO/NOTE: this will not work on sped up AND reversed video
        if (queue.at(i)->pts < queue.at(earliest_frame)->pts) {
            earliest_frame = i;
        }
    }
    av_frame_free(&queue[earliest_frame]);
    queue.removeAt(earliest_frame);
}

Transition* Clip::get_opening_transition() {
    if (opening_transition > -1) {
        if (this->sequence == NULL) {
            return e_clipboard_transitions.at(opening_transition);
        } else {
            return this->sequence->transitions.at(opening_transition);
        }
    }
    return NULL;
}

Transition* Clip::get_closing_transition() {
    if (closing_transition > -1) {
        if (this->sequence == NULL) {
            return e_clipboard_transitions.at(closing_transition);
        } else {
            return this->sequence->transitions.at(closing_transition);
        }
    }
    return NULL;
}

/**
 * @brief get_frame
 * @param playhead
 */
void Clip::get_frame(const long playhead) {
    if (finished_opening) {
        const FootageStream* ms = timeline_info.media->get_object<Footage>()->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream);

        int64_t target_pts = qMax(0L, playhead_to_timestamp(playhead));
        int64_t second_pts = qRound64(av_q2d(av_inv_q(media_handling.stream->time_base)));
        if (ms->video_interlacing != VIDEO_PROGRESSIVE) {
            target_pts *= 2;
            second_pts *= 2;
        }

        AVFrame* target_frame = NULL;

        bool reset = false;
        bool use_cache = true;

        queue_lock.lock();
        if (queue.size() > 0) {
            if (ms->infinite_length) {
                target_frame = queue.at(0);
            } else {
                // correct frame may be somewhere else in the queue
                int closest_frame = 0;

                for (int i=1;i<queue.size();i++) {
                    //dout << "results for" << i << qAbs(queue.at(i)->pts - target_pts) << qAbs(queue.at(closest_frame)->pts - target_pts) << queue.at(i)->pts << target_pts;

                    if (queue.at(i)->pts == target_pts) {
                        closest_frame = i;
                        break;
                    } else if (queue.at(i)->pts > queue.at(closest_frame)->pts && queue.at(i)->pts < target_pts) {
                        closest_frame = i;
                    }
                }

                // remove all frames earlier than this one from the queue
                target_frame = queue.at(closest_frame);
                int64_t next_pts = INT64_MAX;
                int64_t minimum_ts = target_frame->pts;

                int previous_frame_count = 0;
                if (e_config.previous_queue_type == FRAME_QUEUE_TYPE_SECONDS) {
                    minimum_ts -= (second_pts*e_config.previous_queue_size);
                }

                //dout << "closest frame was" << closest_frame << "with" << target_frame->pts << "/" << target_pts;
                for (int i=0;i<queue.size();i++) {
                    if (queue.at(i)->pts > target_frame->pts && queue.at(i)->pts < next_pts) {
                        next_pts = queue.at(i)->pts;
                    }
                    if (queue.at(i) != target_frame && ((queue.at(i)->pts > minimum_ts) == timeline_info.reverse)) {
                        if (e_config.previous_queue_type == FRAME_QUEUE_TYPE_SECONDS) {
                            //dout << "removed frame at" << i << "because its pts was" << queue.at(i)->pts << "compared to" << target_frame->pts;
                            av_frame_free(&queue[i]); // may be a little heavy for the main thread?
                            queue.removeAt(i);
                            i--;
                        } else {
                            // TODO sort from largest to smallest
                            previous_frame_count++;
                        }
                    }
                }

                if (e_config.previous_queue_type == FRAME_QUEUE_TYPE_FRAMES) {
                    while (previous_frame_count > qCeil(e_config.previous_queue_size)) {
                        int smallest = 0;
                        for (int i=1;i<queue.size();i++) {
                            if (queue.at(i)->pts < queue.at(smallest)->pts) {
                                smallest = i;
                            }
                        }
                        av_frame_free(&queue[smallest]);
                        queue.removeAt(smallest);
                        previous_frame_count--;
                    }
                }

                if (next_pts == INT64_MAX) {
                    next_pts = target_frame->pts + target_frame->pkt_duration;
                }

                // we didn't get the exact timestamp
                if (target_frame->pts != target_pts) {
                    if (target_pts > target_frame->pts && target_pts <= next_pts) {
                        //TODO: uhh
                    } else {
                        int64_t pts_diff = qAbs(target_pts - target_frame->pts);
                        if (reached_end && target_pts > target_frame->pts) {
                            reached_end = false;
                            use_cache = false;
                        } else if (target_pts != last_invalid_ts && (target_pts < target_frame->pts || pts_diff > second_pts)) {
                            if (!e_config.fast_seeking) {
                                target_frame = NULL;
                            }
                            reset = true;
                            dinfo << "Resetting";
                            last_invalid_ts = target_pts;
                        } else {
                            if (queue.size() >= max_queue_size) {
                                queue_remove_earliest();
                            }
                            ignore_reverse = true;
                            target_frame = NULL;
                        }
                    }
                }
            }
        } else {
            dinfo << "Resetting";
            reset = true;
        }

        if (target_frame == NULL || reset) {
            // reset cache
            e_texture_failed = true;
            dout << "[INFO] Frame queue couldn't keep up - either the user seeked or the system is overloaded (queue size:" << queue.size() <<  ") " << reset;
        }

        if (target_frame != NULL) {
            int nb_components = av_pix_fmt_desc_get(static_cast<enum AVPixelFormat>(pix_fmt))->nb_components;
            glPixelStorei(GL_UNPACK_ROW_LENGTH, target_frame->linesize[0]/nb_components);

            bool copied = false;
            uint8_t* data = target_frame->data[0];
            int frame_size;

            for (int i=0;i<effects.size();i++) {
                EffectPtr e = effects.at(i);
                if (e->enable_image) {
                    if (!copied) {
                        frame_size = target_frame->linesize[0]*target_frame->height;
                        data = new uint8_t[frame_size];
                        memcpy(data, target_frame->data[0], frame_size);
                        copied = true;
                    }
                    e->process_image(get_timecode(playhead), data, frame_size);
                }
            }

            texture->setData(0, get_gl_pix_fmt_from_av(pix_fmt), QOpenGLTexture::UInt8, data);

            if (copied) delete [] data;

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }

        queue_lock.unlock();

        // get more frames
        QVector<ClipPtr> empty;
        if (use_cache) {
            cache(playhead, reset, false, empty);
        }
    } else {
        dwarning << "Not finished opening";
    }
}


double Clip::get_timecode(const long playhead) {
    return (static_cast<double>(playhead - get_timeline_in_with_transition() + get_clip_in_with_transition())
            / static_cast<double>(sequence->getFrameRate()) );
}

long Clip::get_clip_in_with_transition() {
    if (get_opening_transition() != NULL && get_opening_transition()->secondary_clip != NULL) {
        // we must be the secondary clip, so return (timeline in - length)
        return timeline_info.clip_in - get_opening_transition()->get_true_length();
    }
    return timeline_info.clip_in;
}

long Clip::get_timeline_in_with_transition() {
    if (get_opening_transition() != NULL && get_opening_transition()->secondary_clip != NULL) {
        // we must be the secondary clip, so return (timeline in - length)
        return timeline_info.in - get_opening_transition()->get_true_length();
    }
    return timeline_info.in;
}

long Clip::get_timeline_out_with_transition() {
    if (get_closing_transition() != NULL && get_closing_transition()->secondary_clip != NULL) {
        // we must be the primary clip, so return (timeline out + length2)
        return timeline_info.out + get_closing_transition()->get_true_length();
    } else {
        return timeline_info.out;
    }
}

// timeline functions
long Clip::getLength() {
    return timeline_info.out - timeline_info.in;
}

double Clip::getMediaFrameRate() {
    Q_ASSERT(timeline_info.track < 0);
    if (timeline_info.media != NULL) {
        double rate = timeline_info.media->get_frame_rate(timeline_info.media_stream);
        if (!qIsNaN(rate)) return rate;
    }
    if (sequence != NULL) return sequence->getFrameRate();
    return qSNaN();
}

void Clip::recalculateMaxLength() {
    // TODO: calculated_length on failures
    if (sequence != NULL) {
        double fr = this->sequence->getFrameRate();

        fr /= timeline_info.speed;

        media_handling.calculated_length = LONG_MAX;

        if (timeline_info.media != NULL) {
            switch (timeline_info.media->get_type()) {
            case MEDIA_TYPE_FOOTAGE:
            {
                FootagePtr m = timeline_info.media->get_object<Footage>();
                if (m != NULL) {
                    const FootageStream* ms = m->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream);
                    if (ms != NULL && ms->infinite_length) {
                        media_handling.calculated_length = LONG_MAX;
                    } else {
                        media_handling.calculated_length = m->get_length_in_frames(fr);
                    }
                }
            }
                break;
            case MEDIA_TYPE_SEQUENCE:
            {
                SequencePtr s = timeline_info.media->get_object<Sequence>();
                if (s != NULL) {
                    media_handling.calculated_length = refactor_frame_number(s->getEndFrame(), s->getFrameRate(), fr);
                }
            }
                break;
            default:
                //TODO: log/something
                break;
            }
        }
    }
}

long Clip::getMaximumLength() {
    return media_handling.calculated_length;
}

int Clip::getWidth() {
    if (timeline_info.media == NULL && sequence != NULL) {
        return sequence->getWidth();
    }

    switch (timeline_info.media->get_type()) {
    case MEDIA_TYPE_FOOTAGE:
    {
        const FootageStream* ms = timeline_info.media->get_object<Footage>()->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream);
        if (ms != NULL) {
            return ms->video_width;
        }
        if (sequence != NULL) {
            return sequence->getWidth();
        }
        break;
    }
    case MEDIA_TYPE_SEQUENCE:
    {
        SequencePtr sequenceNow = timeline_info.media->get_object<Sequence>();
        if (sequenceNow != NULL) {
            return sequenceNow->getWidth();
        }
        break;
    }
    default:
        //TODO: log/something
        break;
    }
    return 0;
}

int Clip::getHeight() {
    if ( (timeline_info.media == NULL) && (sequence != NULL) ) {
        return sequence->getHeight();
    }

    switch (timeline_info.media->get_type()) {
    case MEDIA_TYPE_FOOTAGE:
    {
        const FootageStream* ms = timeline_info.media->get_object<Footage>()->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream);
        if (ms != NULL) {
            return ms->video_height;
        }
        if (sequence != NULL) {
            return sequence->getHeight();
        }
        break;
    }
    case MEDIA_TYPE_SEQUENCE:
    {
        SequencePtr s = timeline_info.media->get_object<Sequence>();
        return s->getHeight();
        break;
    }
    default:
        //TODO: log/something
        break;
    }//switch
    return 0;
}

void Clip::refactor_frame_rate(ComboAction* ca, double multiplier, bool change_timeline_points) {
    if (change_timeline_points) {
        if (ca != NULL) {
            move(*ca,
                 qRound(static_cast<double>(timeline_info.in) * multiplier),
                 qRound(static_cast<double>(timeline_info.out) * multiplier),
                 qRound(static_cast<double>(timeline_info.clip_in) * multiplier),
                 timeline_info.track);
        }
    }

    // move keyframes
    for (int i=0;i<effects.size();i++) {
        EffectPtr effectNow = effects.at(i);
        for (int j=0; j<effectNow->row_count();j++) {
            EffectRowPtr effectRowNow = effectNow->row(j);
            for (int l=0; l<effectRowNow->fieldCount(); l++) {
                EffectField* f = effectRowNow->field(l);
                for (int k=0; k<f->keyframes.size(); k++) {
                    ca->append(new SetLong(&f->keyframes[k].time, f->keyframes[k].time, f->keyframes[k].time * multiplier));
                }
            }
        }
    }
}

void Clip::run() {
    // open_lock is used to prevent the clip from being destroyed before the cacher has closed it properly
    lock.lock();
    finished_opening = false;
    is_open = true;
    cache_info.caching = true;
    cache_info.interrupt = false;

    if (open_worker()) {
        dinfo << "Cache thread starting";
        while (cache_info.caching) {
            can_cache.wait(&lock);
            if (!cache_info.caching) {
                break;
            } else {
                while (true) {
                    cache_worker(cache_info.playhead, cache_info.reset, cache_info.scrubbing, cache_info.nests);
                    if (multithreaded && cache_info.interrupt && (timeline_info.track < 0) ) {
                        cache_info.interrupt = false;
                    } else {
                        break;
                    }
                }//while
            }
        }//while

        dwarning << "caching thread stopped";

        close_worker();

        lock.unlock();
        open_lock.unlock();
    } else {
        derror << "Failed to open worker";
    }
}


void Clip::apply_audio_effects(const double timecode_start, AVFrame* frame, const int nb_bytes, QVector<ClipPtr>& nests) {
    // perform all audio effects
    const double timecode_end = timecode_start + bytes_to_seconds(nb_bytes, frame->channels, frame->sample_rate);

    for (int j=0; j < effects.size();j++) {
        EffectPtr e = effects.at(j);
        if (e->is_enabled()) {
            e->process_audio(timecode_start, timecode_end, frame->data[0], nb_bytes, 2);
        }
    }
    if (get_opening_transition() != NULL) {
        if (timeline_info.media != NULL && timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
            const double transition_start = (get_clip_in_with_transition() / sequence->getFrameRate());
            const double transition_end = (get_clip_in_with_transition() + get_opening_transition()->get_length()) / sequence->getFrameRate();
            if (timecode_end < transition_end) {
                const double adjustment = transition_end - transition_start;
                const double adjusted_range_start = (timecode_start - transition_start) / adjustment;
                const double adjusted_range_end = (timecode_end - transition_start) / adjustment;
                get_opening_transition()->process_audio(adjusted_range_start, adjusted_range_end, frame->data[0], nb_bytes, TA_OPENING_TRANSITION);
            }
        }
    }
    if (get_closing_transition() != NULL) {
        if (timeline_info.media != NULL && timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
            const long length_with_transitions = get_timeline_out_with_transition() - get_timeline_in_with_transition();
            const double transition_start = (get_clip_in_with_transition() + length_with_transitions - get_closing_transition()->get_length()) / sequence->getFrameRate();
            const double transition_end = (get_clip_in_with_transition() + length_with_transitions) / sequence->getFrameRate();
            if (timecode_start > transition_start) {
                const double adjustment = transition_end - transition_start;
                const double adjusted_range_start = (timecode_start - transition_start) / adjustment;
                const double adjusted_range_end = (timecode_end - transition_start) / adjustment;
                get_closing_transition()->process_audio(adjusted_range_start, adjusted_range_end, frame->data[0], nb_bytes, TA_CLOSING_TRANSITION);
            }
        }
    }

    //TODO: hmm
//    if (!nests.isEmpty()) {
//        ClipPtr next_nest = nests.last();
//        nests.removeLast();
//        apply_audio_effects(next_nest, timecode_start + (((double)get_timeline_in_with_transition()-get_clip_in_with_transition())/sequence->getFrameRate()), frame, nb_bytes, nests);
//    }
}



long Clip::playhead_to_frame(const long playhead) {
    return (qMax(0L, playhead - get_timeline_in_with_transition()) + get_clip_in_with_transition());
}


int64_t Clip::playhead_to_timestamp(const long playhead) {
    return seconds_to_timestamp(playhead_to_seconds(playhead));
}

bool Clip::retrieve_next_frame(AVFrame* frame) { //TODO: frame could be the same class member over & over
    int result = 0;
    int receive_ret;

    // do we need to retrieve a new packet for a new frame?
    av_frame_unref(frame);
    while ((receive_ret = avcodec_receive_frame(media_handling.codecCtx, frame)) == AVERROR(EAGAIN)) {
        int read_ret = 0;
        do {
            if (pkt_written) {
                av_packet_unref(media_handling.pkt);
                pkt_written = false;
            }
            read_ret = av_read_frame(media_handling.formatCtx, media_handling.pkt);
            if (read_ret >= 0) {
                pkt_written = true;
            }
        } while (read_ret >= 0 && media_handling.pkt->stream_index != timeline_info.media_stream);

        if (read_ret >= 0) {
            int send_ret = avcodec_send_packet(media_handling.codecCtx, media_handling.pkt);
            if (send_ret < 0) {
                dout << "[ERROR] Failed to send packet to decoder." << send_ret;
                return send_ret;
            }
        } else {
            if (read_ret == AVERROR_EOF) {
                int send_ret = avcodec_send_packet(media_handling.codecCtx, NULL);
                if (send_ret < 0) {
                    dout << "[ERROR] Failed to send packet to decoder." << send_ret;
                    return send_ret;
                }
            } else {
                dout << "[ERROR] Could not read frame." << read_ret;
                return read_ret; // skips trying to find a frame at all
            }
        }
    }
    if (receive_ret < 0) {
        if (receive_ret != AVERROR_EOF) dout << "[ERROR] Failed to receive packet from decoder." << receive_ret;
        result = receive_ret;
    }

    return result == 0;
}

/**
 * @brief returns time in seconds
 * @param playhead
 * @return seconds
 */
double Clip::playhead_to_seconds(const long playhead) {
    long clip_frame = playhead_to_frame(playhead);
    if (timeline_info.reverse) {
        clip_frame = getMaximumLength() - clip_frame - 1;
    }
    double secs = (static_cast<double>(clip_frame)/sequence->getFrameRate()) * timeline_info.speed;
    if (timeline_info.media != NULL && timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
        secs *= timeline_info.media->get_object<Footage>()->speed;
    }
    return secs;
}

int64_t Clip::seconds_to_timestamp(const double seconds) {
    return qRound64(seconds * av_q2d(av_inv_q(media_handling.stream->time_base))) + qMax(0L, media_handling.stream->start_time);
}

//TODO: hmmm
void Clip::cache_audio_worker(const bool scrubbing, QVector<ClipPtr> &nests) {
    long timeline_in = get_timeline_in_with_transition();
    long timeline_out = get_timeline_out_with_transition();
    long target_frame = audio_playback.target_frame;

    long frame_skip = 0;
    double last_fr = sequence->getFrameRate();
    if (!nests.isEmpty()) {
        for (int i=nests.size()-1;i>=0;i--) {
            //TODO: do calcs outside of calls
            timeline_in = refactor_frame_number(timeline_in, last_fr,
                                                nests.at(i)->sequence->getFrameRate()) + nests.at(i)->get_timeline_in_with_transition() - nests.at(i)->get_clip_in_with_transition();
            timeline_out = refactor_frame_number(timeline_out, last_fr,
                                                 nests.at(i)->sequence->getFrameRate()) + nests.at(i)->get_timeline_in_with_transition() - nests.at(i)->get_clip_in_with_transition();
            target_frame = refactor_frame_number(target_frame, last_fr,
                                                 nests.at(i)->sequence->getFrameRate()) + nests.at(i)->get_timeline_in_with_transition() - nests.at(i)->get_clip_in_with_transition();

            timeline_out = qMin(timeline_out, nests.at(i)->get_timeline_out_with_transition());

            frame_skip = refactor_frame_number(frame_skip, last_fr, nests.at(i)->sequence->getFrameRate());

            long validator = nests.at(i)->get_timeline_in_with_transition() - timeline_in;
            if (validator > 0) {
                frame_skip += validator;
                //timeline_in = nests.at(i)->get_timeline_in_with_transition();
            }

            last_fr = nests.at(i)->sequence->getFrameRate();
        }
    }

    while (true) {
        AVFrame* av_frame;
        int nb_bytes = INT_MAX;

        if (timeline_info.media == NULL) {
            av_frame = media_handling.frame;
            nb_bytes = av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels;
            while ((audio_playback.frame_sample_index == -1 || audio_playback.frame_sample_index >= nb_bytes) && nb_bytes > 0) {
                // create "new frame"
                memset(media_handling.frame->data[0], 0, nb_bytes);
                apply_audio_effects(bytes_to_seconds(av_frame->pts, av_frame->channels, av_frame->sample_rate), av_frame, nb_bytes, nests);
                media_handling.frame->pts += nb_bytes;
                audio_playback.frame_sample_index = 0;
                if (audio_playback.buffer_write == 0) {
                    audio_playback.buffer_write = get_buffer_offset_from_frame(last_fr, qMax(timeline_in, target_frame));
                }
                int offset = audio_ibuffer_read - audio_playback.buffer_write;
                if (offset > 0) {
                    audio_playback.buffer_write += offset;
                    audio_playback.frame_sample_index += offset;
                }
            }
        } else if (timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
            double timebase = av_q2d(media_handling.stream->time_base);

            av_frame = queue.at(0);

            // retrieve frame
            bool new_frame = false;
            while ((audio_playback.frame_sample_index == -1 || audio_playback.frame_sample_index >= nb_bytes) && nb_bytes > 0) {
                // no more audio left in frame, get a new one
                if (!reached_end) {
                    int loop = 0;

                    if (timeline_info.reverse && !audio_playback.just_reset) {
                        avcodec_flush_buffers(media_handling.codecCtx);
                        reached_end = false;
                        int64_t backtrack_seek = qMax(audio_playback.reverse_target - static_cast<int64_t>(av_q2d(av_inv_q(media_handling.stream->time_base))), static_cast<int64_t>(0));
                        av_seek_frame(media_handling.formatCtx, media_handling.stream->index, backtrack_seek, AVSEEK_FLAG_BACKWARD);
#ifdef AUDIOWARNINGS
                        if (backtrack_seek == 0) {
                            dout << "backtracked to 0";
                        }
#endif
                    }

                    do {
                        av_frame_unref(av_frame);

                        int ret;

                        while ((ret = av_buffersink_get_frame(buffersink_ctx, av_frame)) == AVERROR(EAGAIN)) {
                            ret = retrieve_next_frame(media_handling.frame);
                            if (ret >= 0) {
                                if ((ret = av_buffersrc_add_frame_flags(buffersrc_ctx, media_handling.frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
                                    dout << "[ERROR] Could not feed filtergraph -" << ret;
                                    break;
                                }
                            } else {
                                if (ret == AVERROR_EOF) {
#ifdef AUDIOWARNINGS
                                        dout << "reached EOF while reading";
#endif
                                    // TODO revise usage of reached_end in audio
                                    if (!timeline_info.reverse) {
                                        reached_end = true;
                                    } else {
                                    }
                                } else {
                                    dout << "[WARNING] Raw audio frame data could not be retrieved." << ret;
                                    reached_end = true;
                                }
                                break;
                            }
                        }

                        if (ret < 0) {
                            if (ret != AVERROR_EOF) {
                                dout << "[ERROR] Could not pull from filtergraph";
                                reached_end = true;
                                break;
                            } else {
#ifdef AUDIOWARNINGS
                                dout << "reached EOF while pulling from filtergraph";
#endif
                                if (!timeline_info.reverse) break;
                            }
                        }

                        if (timeline_info.reverse) {
                            if (loop > 1) {
                                AVFrame* rev_frame = queue.at(1);
                                if (ret != AVERROR_EOF) {
                                    if (loop == 2) {
#ifdef AUDIOWARNINGS
                                        dout << "starting rev_frame";
#endif
                                        rev_frame->nb_samples = 0;
                                        rev_frame->pts = media_handling.frame->pkt_pts;
                                    }
                                    int offset = rev_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(rev_frame->format)) * rev_frame->channels;
#ifdef AUDIOWARNINGS
                                    dout << "offset 1:" << offset;
                                    dout << "retrieved samples:" << frame->nb_samples << "size:" << (frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format)) * frame->channels);
#endif
                                    memcpy(
                                            rev_frame->data[0]+offset,
                                            av_frame->data[0],
                                            (av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels)
                                        );
#ifdef AUDIOWARNINGS
                                    dout << "pts:" << media_handling.frame->pts << "dur:" << media_handling.frame->pkt_duration << "rev_target:" << audio_playback.reverse_target << "offset:" << offset << "limit:" << rev_frame->linesize[0];
#endif
                                }

                                rev_frame->nb_samples += av_frame->nb_samples;

                                if ((media_handling.frame->pts >= audio_playback.reverse_target) || (ret == AVERROR_EOF)) {
/*
#ifdef AUDIOWARNINGS
                                    dout << "time for the end of rev cache" << rev_frame->nb_samples << rev_target << media_handling.frame->pts << media_handling.frame->pkt_duration << media_handling.frame->nb_samples;
                                    dout << "diff:" << (media_handling.frame->pkt_pts + media_handling.frame->pkt_duration) - rev_target;
#endif
                                    int cutoff = qRound64((((media_handling.frame->pkt_pts + media_handling.frame->pkt_duration) - audio_playback.reverse_target) * timebase) * audio_output->format().sampleRate());
                                    if (cutoff > 0) {
#ifdef AUDIOWARNINGS
                                        dout << "cut off" << cutoff << "samples (rate:" << audio_output->format().sampleRate() << ")";
#endif
                                        rev_frame->nb_samples -= cutoff;
                                    }
*/

#ifdef AUDIOWARNINGS
                                    dout << "pre cutoff deets::: rev_frame.pts:" << rev_frame->pts << "rev_frame.nb_samples" << rev_frame->nb_samples << "rev_target:" << audio_playback.reverse_target;
#endif
                                    double playback_speed = timeline_info.speed * timeline_info.media->get_object<Footage>()->speed;
                                    rev_frame->nb_samples = qRound64(static_cast<double>(audio_playback.reverse_target - rev_frame->pts) / media_handling.stream->codecpar->sample_rate * (current_audio_freq() / playback_speed));
#ifdef AUDIOWARNINGS
                                    dout << "post cutoff deets::" << rev_frame->nb_samples;
#endif

                                    int frame_size = rev_frame->nb_samples * rev_frame->channels * av_get_bytes_per_sample(static_cast<AVSampleFormat>(rev_frame->format));
                                    int half_frame_size = frame_size >> 1;

                                    int sample_size = rev_frame->channels*av_get_bytes_per_sample(static_cast<AVSampleFormat>(rev_frame->format));
                                    char* temp_chars = new char[sample_size];
                                    for (int i=0;i<half_frame_size;i+=sample_size) {
                                        for (int j=0;j<sample_size;j++) {
                                            temp_chars[j] = rev_frame->data[0][i+j];
                                        }
                                        for (int j=0;j<sample_size;j++) {
                                            rev_frame->data[0][i+j] = rev_frame->data[0][frame_size-i-sample_size+j];
                                        }
                                        for (int j=0;j<sample_size;j++) {
                                            rev_frame->data[0][frame_size-i-sample_size+j] = temp_chars[j];
                                        }
                                    }
                                    delete [] temp_chars;

                                    audio_playback.reverse_target = rev_frame->pts;
                                    av_frame = rev_frame;
                                    break;
                                }
                            }

                            loop++;

#ifdef AUDIOWARNINGS
                            dout << "loop" << loop;
#endif
                        } else {
                            av_frame->pts = media_handling.frame->pts;
                            break;
                        }
                    } while (true);
                } else {
                    // if there is no more data in the file, we flush the remainder out of swresample
                    break;
                }

                new_frame = true;

                if (audio_playback.frame_sample_index < 0) {
                    audio_playback.frame_sample_index = 0;
                } else {
                    audio_playback.frame_sample_index -= nb_bytes;
                }

                nb_bytes = av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels;

                if (audio_playback.just_reset) {
                    // get precise sample offset for the elected clip_in from this audio frame
                    double target_sts = playhead_to_seconds(audio_playback.target_frame);
                    double frame_sts = ((av_frame->pts - media_handling.stream->start_time) * timebase);
                    int nb_samples = qRound64((target_sts - frame_sts)*current_audio_freq());
                    audio_playback.frame_sample_index = nb_samples * 4;
#ifdef AUDIOWARNINGS
                    dout << "fsts:" << frame_sts << "tsts:" << target_sts << "nbs:" << nb_samples << "nbb:" << nb_bytes << "rev_targetToSec:" << (audio_playback.reverse_target * timebase);
                    dout << "fsi-calc:" << audio_playback.frame_sample_index;
#endif
                    if (timeline_info.reverse) audio_playback.frame_sample_index = nb_bytes - audio_playback.frame_sample_index;
                    audio_playback.just_reset = false;
                }

#ifdef AUDIOWARNINGS
                dout << "fsi-post-post:" << audio_playback.frame_sample_index;
#endif
                if (audio_playback.buffer_write == 0) {
                    audio_playback.buffer_write = get_buffer_offset_from_frame(last_fr, qMax(timeline_in, target_frame));

                    if (frame_skip > 0) {
                        int target = get_buffer_offset_from_frame(last_fr, qMax(timeline_in + frame_skip, target_frame));
                        audio_playback.frame_sample_index += (target - audio_playback.buffer_write);
                        audio_playback.buffer_write = target;
                    }
                }

                int offset = audio_ibuffer_read - audio_playback.buffer_write;
                if (offset > 0) {
                    audio_playback.buffer_write += offset;
                    audio_playback.frame_sample_index += offset;
                }

                // try to correct negative fsi
                if (audio_playback.frame_sample_index < 0) {
                    audio_playback.buffer_write -= audio_playback.frame_sample_index;
                    audio_playback.frame_sample_index = 0;
                }
            }

            if (timeline_info.reverse) av_frame = queue.at(1);

#ifdef AUDIOWARNINGS
            dout << "j" << audio_playback.frame_sample_index << nb_bytes;
#endif

            // apply any audio effects to the data
            if (nb_bytes == INT_MAX) nb_bytes = av_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(av_frame->format)) * av_frame->channels;
            if (new_frame) {
                apply_audio_effects(bytes_to_seconds(audio_playback.buffer_write, 2, current_audio_freq()) + audio_ibuffer_timecode + ((double)get_clip_in_with_transition()/sequence->getFrameRate()) - ((double)timeline_in/last_fr), av_frame, nb_bytes, nests);
            }
        } else {
            // shouldn't ever get here
            dout << "[ERROR] Tried to cache a non-footage/tone clip";
            return;
        }

        // mix audio into internal buffer
        if (av_frame->nb_samples == 0) {
            break;
        } else {
            long buffer_timeline_out = get_buffer_offset_from_frame(sequence->getFrameRate(), timeline_out);
            audio_write_lock.lock();

            while (audio_playback.frame_sample_index < nb_bytes
                   && audio_playback.buffer_write < audio_ibuffer_read+(audio_ibuffer_size>>1)
                   && audio_playback.buffer_write < buffer_timeline_out) {
                int upper_byte_index = (audio_playback.buffer_write+1)%audio_ibuffer_size;
                int lower_byte_index = (audio_playback.buffer_write)%audio_ibuffer_size;
                qint16 old_sample = static_cast<qint16>((audio_ibuffer[upper_byte_index] & 0xFF) << 8 | (audio_ibuffer[lower_byte_index] & 0xFF));
                qint16 new_sample = static_cast<qint16>((av_frame->data[0][audio_playback.frame_sample_index+1] & 0xFF) << 8 | (av_frame->data[0][audio_playback.frame_sample_index] & 0xFF));
                qint16 mixed_sample = mix_audio_sample(old_sample, new_sample);

                audio_ibuffer[upper_byte_index] = static_cast<quint8>((mixed_sample >> 8) & 0xFF);
                audio_ibuffer[lower_byte_index] = static_cast<quint8>(mixed_sample & 0xFF);

                audio_playback.buffer_write+=2;
                audio_playback.frame_sample_index+=2;
            }

#ifdef AUDIOWARNINGS
            if (audio_playback.buffer_write >= buffer_timeline_out) dout << "timeline out at fsi" << audio_playback.frame_sample_index << "of frame ts" << media_handling.frame->pts;
#endif

            audio_write_lock.unlock();

            if (scrubbing) {
                if (audio_thread != NULL) audio_thread->notifyReceiver();
            }

            if (audio_playback.frame_sample_index == nb_bytes) {
                audio_playback.frame_sample_index = -1;
            } else {
                // assume we have no more data to send
                break;
            }

//			dout << "ended" << audio_playback.frame_sample_index << nb_bytes;
        }
        if (reached_end) {
            av_frame->nb_samples = 0;
        }
        if (scrubbing) {
            break;
        }
    }

    QMetaObject::invokeMethod(e_panel_footage_viewer, "play_wake", Qt::QueuedConnection);
    QMetaObject::invokeMethod(e_panel_sequence_viewer, "play_wake", Qt::QueuedConnection);
}

void Clip::cache_video_worker(const long playhead) {
    int read_ret, send_ret, retr_ret;

    int64_t target_pts = seconds_to_timestamp(playhead_to_seconds(playhead));

    int limit = max_queue_size;
    if (ignore_reverse) {
        // waiting for one frame
        limit = queue.size() + 1;
    } else if (timeline_info.reverse) {
        limit *= 2;
    }

    if (queue.size() < limit) {
        bool reverse = (timeline_info.reverse && !ignore_reverse);
        ignore_reverse = false;

        int64_t smallest_pts = INT64_MAX;
        if (reverse && queue.size() > 0) {
            int64_t quarter_sec = qRound64(av_q2d(av_inv_q(media_handling.stream->time_base))) >> 2;
            for (int i=0;i<queue.size();i++) {
                smallest_pts = qMin(smallest_pts, queue.at(i)->pts);
            }
            avcodec_flush_buffers(media_handling.codecCtx);
            reached_end = false;
            int64_t seek_ts = qMax(static_cast<int64_t>(0), smallest_pts - quarter_sec);
            av_seek_frame(media_handling.formatCtx, media_handling.stream->index, seek_ts, AVSEEK_FLAG_BACKWARD);
        } else {
            smallest_pts = target_pts;
        }

        if (multithreaded && cache_info.interrupt) { // ignore interrupts for now
            cache_info.interrupt = false;
        }

        while (true) {
            AVFrame* frame = av_frame_alloc();

            FootagePtr media = timeline_info.media->get_object<Footage>();
            const FootageStream* ms = media->get_stream_from_file_index(true, timeline_info.media_stream);

            while ((retr_ret = av_buffersink_get_frame(buffersink_ctx, frame)) == AVERROR(EAGAIN)) {
                if (multithreaded && cache_info.interrupt) return; // abort

                AVFrame* send_frame = media_handling.frame;
//				qint64 time = QDateTime::currentMSecsSinceEpoch();
                read_ret = (use_existing_frame) ? 0 : retrieve_next_frame(send_frame);
//				dout << QDateTime::currentMSecsSinceEpoch() - time;
                use_existing_frame = false;
                if (read_ret >= 0) {
                    bool send_it = true;

                    /*if (reverse) {
                        send_it = true;
                    } else if (send_frame->pts > target_pts - eighth_second) {
                        send_it = true;
                    } else if (media->get_stream_from_file_index(true, timeline_info.media_stream)->infinite_length) {
                        send_it = true;
                    } else {
                        dout << "skipped adding a frame to the queue - fpts:" << send_frame->pts << "target:" << target_pts;
                    }*/

                    if (send_it) {
                        if ((send_ret = av_buffersrc_add_frame_flags(buffersrc_ctx, send_frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
                            dout << "[ERROR] Failed to add frame to buffer source." << send_ret;
                            break;
                        }
                    }

                    av_frame_unref(media_handling.frame);
                } else {
                    if (read_ret == AVERROR_EOF) {
                        reached_end = true;
                    } else {
                        dout << "[ERROR] Failed to read frame." << read_ret;
                    }
                    break;
                }
            }

            if (retr_ret < 0) {
                if (retr_ret == AVERROR_EOF) {
                    reached_end = true;
                } else {
                    dout << "[ERROR] Failed to retrieve frame from buffersink." << retr_ret;
                }
                av_frame_free(&frame);
                break;
            } else {
                if (reverse && ((smallest_pts == target_pts && frame->pts >= smallest_pts) || (smallest_pts != target_pts && frame->pts > smallest_pts))) {
                    av_frame_free(&frame);
                    break;
                } else {
                    // thread-safety while adding frame to the queue
                    queue_lock.lock();
                    queue.append(frame);

                    if (!ms->infinite_length && !reverse && queue.size() == limit) {
                        // see if we got the frame we needed (used for speed ups primarily)
                        bool found = false;
                        for (int i=0;i<queue.size();i++) {
                            if (queue.at(i)->pts >= target_pts) {
                                found = true;
                                break;
                            }
                        }
                        if (found) {
                            queue_lock.unlock();
                            break;
                        } else {
                            // remove earliest frame and loop to store another
                            queue_remove_earliest();
                        }
                    }
                    queue_lock.unlock();
                }
            }

            if (multithreaded && cache_info.interrupt) { // abort
                return;
            }
        }
    }
}

/**
 * @brief To set up the caching thread?
 * @param playhead
 * @param reset
 * @param scrubbing
 * @param nests
 */
void Clip::cache_worker(const long playhead, const bool reset, const bool scrubbing, QVector<ClipPtr>& nests) {
    if (reset) {
        // note: for video, playhead is in "internal clip" frames - for audio, it's the timeline playhead
        reset_cache(playhead);
        audio_playback.reset = false;
    }

    if (timeline_info.media == NULL) {
        if (timeline_info.track >= 0) {
            cache_audio_worker(scrubbing, nests);
        }
    } else if (timeline_info.media->get_type() == MEDIA_TYPE_FOOTAGE) {
        if (media_handling.stream != NULL) {
            if (media_handling.stream->codecpar != NULL) {
                if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    cache_video_worker(playhead);
                } else if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                    cache_audio_worker(scrubbing, nests);
                }
            }
        }
    }
}

/**
 * @brief reset_cache
 * @param target_frame
 */
void Clip::reset_cache(const long target_frame) {
    // if we seek to a whole other place in the timeline, we'll need to reset the cache with new values
    if (timeline_info.media == NULL) {
        if (timeline_info.track >= 0) {
            // tone clip
            reached_end = false;
            audio_playback.target_frame = target_frame;
            audio_playback.frame_sample_index = -1;
            media_handling.frame->pts = 0;
        }
    } else {
        const FootageStream* ms = timeline_info.media->get_object<Footage>()->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream);
        if (ms->infinite_length) {
            /*avcodec_flush_buffers(media_handling.codecCtx);
            av_seek_frame(media_handling.formatCtx, ms->file_index, 0, AVSEEK_FLAG_BACKWARD);*/
            use_existing_frame = false;
        } else {
            if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                // clear current queue
                queue_clear();

                // seeks to nearest keyframe (target_frame represents internal clip frame)
                int64_t target_ts = seconds_to_timestamp(playhead_to_seconds(target_frame));
                int64_t seek_ts = target_ts;
                int64_t timebase_half_second = qRound64(av_q2d(av_inv_q(media_handling.stream->time_base)));
                if (timeline_info.reverse) seek_ts -= timebase_half_second;

                while (true) {
                    // flush ffmpeg codecs
                    avcodec_flush_buffers(media_handling.codecCtx);
                    reached_end = false;

                    if (seek_ts > 0) {
                        av_seek_frame(media_handling.formatCtx, ms->file_index, seek_ts, AVSEEK_FLAG_BACKWARD);

                        av_frame_unref(media_handling.frame);
                        int ret = retrieve_next_frame(media_handling.frame);
                        if (ret < 0) {
                            dout << "[WARNING] Seeking terminated prematurely";
                            break;
                        }
                        if (media_handling.frame->pts <= target_ts) {
                            use_existing_frame = true;
                            break;
                        } else {
                            seek_ts -= timebase_half_second;
                        }
                    } else {
                        av_frame_unref(media_handling.frame);
                        av_seek_frame(media_handling.formatCtx, ms->file_index, 0, AVSEEK_FLAG_BACKWARD);
                        use_existing_frame = false;
                        break;
                    }
                }
            } else if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                // flush ffmpeg codecs
                avcodec_flush_buffers(media_handling.codecCtx);
                reached_end = false;

                // seek (target_frame represents timeline timecode in frames, not clip timecode)

                int64_t timestamp = seconds_to_timestamp(playhead_to_seconds(target_frame));

                if (timeline_info.reverse) {
                    audio_playback.reverse_target = timestamp;
                    timestamp -= av_q2d(av_inv_q(media_handling.stream->time_base));
#ifdef AUDIOWARNINGS
                    dout << "seeking to" << timestamp << "(originally" << audio_playback.reverse_target << ")";
                } else {
                    dout << "reset called; seeking to" << timestamp;
#endif
                }
                av_seek_frame(media_handling.formatCtx, ms->file_index, timestamp, AVSEEK_FLAG_BACKWARD);
                audio_playback.target_frame = target_frame;
                audio_playback.frame_sample_index = -1;
                audio_playback.just_reset = true;
            }
        }
    }
}


//TODO: use of pointers is causing holdup
void Clip::move(ComboAction &ca, const long iin, const long iout,
                const long iclip_in, const int itrack, const bool verify_transitions,
                const bool relative)
{
    //TODO: check the ClipPtr(this) does what intended
    ca.append(new MoveClipAction(ClipPtr(this), iin, iout, iclip_in, itrack, relative));

    if (verify_transitions) {
        if ( (get_opening_transition() != NULL) &&
             (get_opening_transition()->secondary_clip != NULL) &&
             (get_opening_transition()->secondary_clip->timeline_info.out != iin) ) {
            // separate transition
            //            ca.append(new SetPointer((void**) &get_opening_transition()->secondary_clip, NULL));
            ca.append(new AddTransitionCommand(get_opening_transition()->secondary_clip, NULL, get_opening_transition(), NULL, TA_CLOSING_TRANSITION, 0));
        }

        if ( (get_closing_transition() != NULL) &&
             (get_closing_transition()->secondary_clip != NULL) &&
             (get_closing_transition()->parent_clip->timeline_info.in != iout) ) {
            // separate transition
            //            ca.append(new SetPointer((void**) &get_closing_transition()->secondary_clip, NULL));
            ca.append(new AddTransitionCommand(ClipPtr(this), NULL, get_closing_transition(), NULL, TA_CLOSING_TRANSITION, 0));
        }
    }
}

