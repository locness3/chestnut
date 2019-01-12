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
#include "playback/cacher.h"
#include "playback/audio.h"
#include "panels/project.h"
#include "project/sequence.h"
#include "panels/timeline.h"
#include "project/media.h"
#include "io/clipboard.h"
#include "undo.h"
#include "debug.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

namespace {
    const bool WAIT_ON_CLOSE = true;
    const AVSampleFormat SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;
    const int AUDIO_SAMPLES = 2048;
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
    if (open) {
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
        //        copy->effects.append(effects.at(i)->copy(copy)); //TODO:hmm
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

    dout << "[INFO] Clip opened on track" << timeline_info.track;
    return false;
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

    dout << "[INFO] Clip closed on track" << timeline_info.track;
}


/**
 * @brief Close this clip and free up resources
 * @param wait  Wait on cache?
 */
void Clip::close(const bool wait) {
    if (open) {
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
                cacher->caching = false;
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

            open = false;
        }
    }
}

void Clip::reset() {
    audio_playback.just_reset = false;
    open = false;
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
            if (c != NULL) c->reset_audio();
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
            //            ca.append(new SetPointer((void**) &c->get_opening_transition()->secondary_clip, NULL));
            ca.append(new AddTransitionCommand(get_opening_transition()->secondary_clip, NULL, get_opening_transition(), NULL, TA_CLOSING_TRANSITION, 0));
        }

        if ( (get_closing_transition() != NULL) &&
             (get_closing_transition()->secondary_clip != NULL) &&
             (get_closing_transition()->parent_clip->timeline_info.in != iout) ) {
            // separate transition
            //            ca.append(new SetPointer((void**) &c->get_closing_transition()->secondary_clip, NULL));
            ca.append(new AddTransitionCommand(ClipPtr(this), NULL, get_closing_transition(), NULL, TA_CLOSING_TRANSITION, 0));
        }
    }
}

