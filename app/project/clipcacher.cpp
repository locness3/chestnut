#include "clipcacher.h"

#include "debug.h"

ClipCacher::ClipCacher(std::shared_ptr<Clip> clipCached, QObject *parent)
    : QObject(parent),
      m_cachedClip(clipCached)
{

}

ClipCacher::~ClipCacher()
{
    m_nests.clear();
}



void ClipCacher::doWork(const TimeLineInfo &info)
{
    //    finished_opening = false;
    //    is_open = true;
    m_caching = true;
    m_interrupt = false;

    if (initialise(info))
    {
        qInfo() << "Clip Caching thread starting";
        while (m_caching) {
            m_canCache.wait(&m_lock);
            if (!m_caching) {
                break;
            }
            while (true) {
                worker(m_playhead, m_reset, m_scrubbing, m_nests);
                if (m_interrupt && (info.track < 0)) {
                    m_interrupt = false;
                } else {
                    break;
                }
            }//while
        }//while

        qWarning() << "Caching thread stopped";

        uninitialise();

        m_lock.unlock();
    } else {
        qCritical() << "Failed to open worker";
    }
    emit workDone();
}


void ClipCacher::update(const long playhead, const bool resetCache, const bool scrubbing, QVector<std::shared_ptr<Clip>>& nests)
{
    // TODO: lock
    m_playhead = playhead;
    m_reset = resetCache;
    m_scrubbing = scrubbing;
    m_nests = nests;
}

bool ClipCacher::initialise(const TimeLineInfo& info)
{
    if (info.media) {
        if (info.track >= 0) {

        }
    }
    //    if (timeline_info.media == nullptr) {
    //        if (timeline_info.track >= 0) {
    //            media_handling.frame = av_frame_alloc();
    //            media_handling.frame->format = SAMPLE_FORMAT;
    //            media_handling.frame->channel_layout = sequence->getAudioLayout();
    //            media_handling.frame->channels = av_get_channel_layout_nb_channels(media_handling.frame->channel_layout);
    //            media_handling.frame->sample_rate = current_audio_freq();
    //            media_handling.frame->nb_samples = AUDIO_SAMPLES;
    //            av_frame_make_writable(media_handling.frame);
    //            if (av_frame_get_buffer(media_handling.frame, 0)) {
    //                dout << "[ERROR] Could not allocate buffer for tone clip";
    //            }
    //            audio_playback.reset = true;
    //        }
    //    } else if (timeline_info.media->get_type() == MediaType::FOOTAGE) {
    //        // opens file resource for FFmpeg and prepares Clip struct for playback
    //        auto ftg = timeline_info.media->get_object<Footage>();
    //        const char* const filename = ftg->url.toUtf8().data();

    //        FootageStream ms;
    //        if (!ftg->get_stream_from_file_index(timeline_info.track < 0, timeline_info.media_stream, ms)) {
    //            return false;
    //        }
    //        int errCode = avformat_open_input(
    //                    &media_handling.formatCtx,
    //                    filename,
    //                    nullptr,
    //                    nullptr
    //                    );
    //        if (errCode != 0) {
    //            char err[1024];
    //            av_strerror(errCode, err, 1024);
    //            dout << "[ERROR] Could not open" << filename << "-" << err;
    //            return false;
    //        }

    //        errCode = avformat_find_stream_info(media_handling.formatCtx, nullptr);
    //        if (errCode < 0) {
    //            char err[1024];
    //            av_strerror(errCode, err, 1024);
    //            dout << "[ERROR] Could not open" << filename << "-" << err;
    //            return false;
    //        }

    //        av_dump_format(media_handling.formatCtx, 0, filename, 0);

    //        media_handling.stream = media_handling.formatCtx->streams[ms.file_index];
    //        media_handling.codec = avcodec_find_decoder(media_handling.stream->codecpar->codec_id);
    //        media_handling.codecCtx = avcodec_alloc_context3(media_handling.codec);
    //        avcodec_parameters_to_context(media_handling.codecCtx, media_handling.stream->codecpar);

    //        if (ms.infinite_length) {
    //            max_queue_size = 1;
    //        } else {
    //            max_queue_size = 0;
    //            if (e_config.upcoming_queue_type == FRAME_QUEUE_TYPE_FRAMES) {
    //                max_queue_size += qCeil(e_config.upcoming_queue_size);
    //            } else {
    //                max_queue_size += qCeil(ms.video_frame_rate * ftg->speed * e_config.upcoming_queue_size);
    //            }
    //            if (e_config.previous_queue_type == FRAME_QUEUE_TYPE_FRAMES) {
    //                max_queue_size += qCeil(e_config.previous_queue_size);
    //            } else {
    //                max_queue_size += qCeil(ms.video_frame_rate * ftg->speed * e_config.previous_queue_size);
    //            }
    //        }

    //        if (ms.video_interlacing != VIDEO_PROGRESSIVE) max_queue_size *= 2;

    //        media_handling.opts = nullptr;

    //        // optimized decoding settings
    //        if ((media_handling.stream->codecpar->codec_id != AV_CODEC_ID_PNG &&
    //             media_handling.stream->codecpar->codec_id != AV_CODEC_ID_APNG &&
    //             media_handling.stream->codecpar->codec_id != AV_CODEC_ID_TIFF &&
    //             media_handling.stream->codecpar->codec_id != AV_CODEC_ID_PSD)
    //                || !e_config.disable_multithreading_for_images) {
    //            av_dict_set(&media_handling.opts, "threads", "auto", 0);
    //        }
    //        if (media_handling.stream->codecpar->codec_id == AV_CODEC_ID_H264) {
    //            av_dict_set(&media_handling.opts, "tune", "fastdecode", 0);
    //            av_dict_set(&media_handling.opts, "tune", "zerolatency", 0);
    //        }

    //        // Open codec
    //        if (avcodec_open2(media_handling.codecCtx, media_handling.codec, &media_handling.opts) < 0) {
    //            qCritical() << "Could not open codec";
    //        }

    //        // allocate filtergraph
    //        filter_graph = avfilter_graph_alloc();
    //        if (filter_graph == nullptr) {
    //            qCritical() << "Could not create filtergraph";
    //        }
    //        char filter_args[512];

    //        if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    //            snprintf(filter_args, sizeof(filter_args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
    //                     media_handling.stream->codecpar->width,
    //                     media_handling.stream->codecpar->height,
    //                     media_handling.stream->codecpar->format,
    //                     media_handling.stream->time_base.num,
    //                     media_handling.stream->time_base.den,
    //                     media_handling.stream->codecpar->sample_aspect_ratio.num,
    //                     media_handling.stream->codecpar->sample_aspect_ratio.den
    //                     );

    //            avfilter_graph_create_filter(&buffersrc_ctx, avfilter_get_by_name("buffer"), "in", filter_args, nullptr, filter_graph);
    //            avfilter_graph_create_filter(&buffersink_ctx, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, filter_graph);

    //            AVFilterContext* last_filter = buffersrc_ctx;

    //            if (ms.video_interlacing != VIDEO_PROGRESSIVE) {
    //                AVFilterContext* yadif_filter;
    //                char yadif_args[100];
    //                snprintf(yadif_args, sizeof(yadif_args), "mode=3:parity=%d", ((ms.video_interlacing == VIDEO_TOP_FIELD_FIRST) ? 0 : 1)); // there's a CUDA version if we start using nvdec/nvenc
    //                avfilter_graph_create_filter(&yadif_filter, avfilter_get_by_name("yadif"), "yadif", yadif_args, nullptr, filter_graph);

    //                avfilter_link(last_filter, 0, yadif_filter, 0);
    //                last_filter = yadif_filter;
    //            }

    //            /* stabilization code */
    //            bool stabilize = false;
    //            if (stabilize) {
    //                AVFilterContext* stab_filter;
    //                int stab_ret = avfilter_graph_create_filter(&stab_filter, avfilter_get_by_name("vidstabtransform"), "vidstab", "input=/media/matt/Home/samples/transforms.trf", nullptr, filter_graph);
    //                if (stab_ret < 0) {
    //                    char err[100];
    //                    av_strerror(stab_ret, err, sizeof(err));
    //                } else {
    //                    avfilter_link(last_filter, 0, stab_filter, 0);
    //                    last_filter = stab_filter;
    //                }
    //            }

    //            enum AVPixelFormat valid_pix_fmts[] = {
    //                AV_PIX_FMT_RGB24,
    //                AV_PIX_FMT_RGBA,
    //                AV_PIX_FMT_NONE
    //            };

    //            pix_fmt = avcodec_find_best_pix_fmt_of_list(valid_pix_fmts, static_cast<enum AVPixelFormat>(media_handling.stream->codecpar->format), 1, nullptr);
    //            const char* chosen_format = av_get_pix_fmt_name(static_cast<enum AVPixelFormat>(pix_fmt));
    //            char format_args[100];
    //            snprintf(format_args, sizeof(format_args), "pix_fmts=%s", chosen_format);

    //            AVFilterContext* format_conv;
    //            avfilter_graph_create_filter(&format_conv, avfilter_get_by_name("format"), "fmt", format_args, nullptr, filter_graph);
    //            avfilter_link(last_filter, 0, format_conv, 0);

    //            avfilter_link(format_conv, 0, buffersink_ctx, 0);

    //            avfilter_graph_config(filter_graph, nullptr);
    //        } else if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
    //            if (media_handling.codecCtx->channel_layout == 0) media_handling.codecCtx->channel_layout = av_get_default_channel_layout(media_handling.stream->codecpar->channels);

    //            // set up cache
    //            queue.append(av_frame_alloc());
    //            if (timeline_info.reverse) {
    //                AVFrame* reverse_frame = av_frame_alloc();

    //                reverse_frame->format = SAMPLE_FORMAT;
    //                reverse_frame->nb_samples = current_audio_freq()*2;
    //                reverse_frame->channel_layout = sequence->getAudioLayout();
    //                reverse_frame->channels = av_get_channel_layout_nb_channels(sequence->getAudioLayout());
    //                av_frame_get_buffer(reverse_frame, 0);

    //                queue.append(reverse_frame);
    //            }

    //            snprintf(filter_args, sizeof(filter_args), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
    //                     media_handling.stream->time_base.num,
    //                     media_handling.stream->time_base.den,
    //                     media_handling.stream->codecpar->sample_rate,
    //                     av_get_sample_fmt_name(media_handling.codecCtx->sample_fmt),
    //                     media_handling.codecCtx->channel_layout
    //                     );

    //            avfilter_graph_create_filter(&buffersrc_ctx, avfilter_get_by_name("abuffer"), "in", filter_args, nullptr, filter_graph);
    //            avfilter_graph_create_filter(&buffersink_ctx, avfilter_get_by_name("abuffersink"), "out", nullptr, nullptr, filter_graph);

    //            enum AVSampleFormat sample_fmts[] = { SAMPLE_FORMAT,  static_cast<AVSampleFormat>(-1) };
    //            if (av_opt_set_int_list(buffersink_ctx, "sample_fmts", sample_fmts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
    //                dout << "[ERROR] Could not set output sample format";
    //            }

    //            int64_t channel_layouts[] = { AV_CH_LAYOUT_STEREO, static_cast<AVSampleFormat>(-1) };
    //            if (av_opt_set_int_list(buffersink_ctx, "channel_layouts", channel_layouts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
    //                dout << "[ERROR] Could not set output sample format";
    //            }

    //            int target_sample_rate = current_audio_freq();

    //            double playback_speed = timeline_info.speed * ftg->speed;

    //            if (qFuzzyCompare(playback_speed, 1.0)) {
    //                avfilter_link(buffersrc_ctx, 0, buffersink_ctx, 0);
    //            } else if (timeline_info.maintain_audio_pitch) {
    //                AVFilterContext* previous_filter = buffersrc_ctx;
    //                AVFilterContext* last_filter = buffersrc_ctx;

    //                char speed_param[10];

    //                //				if (playback_speed != 1.0) {
    //                double base = (playback_speed > 1.0) ? 2.0 : 0.5;

    //                double speedlog = log(playback_speed) / log(base);
    //                int whole2 = qFloor(speedlog);
    //                speedlog -= whole2;

    //                if (whole2 > 0) {
    //                    snprintf(speed_param, sizeof(speed_param), "%f", base);
    //                    for (int i=0;i<whole2;i++) {
    //                        AVFilterContext* tempo_filter = nullptr;
    //                        avfilter_graph_create_filter(&tempo_filter, avfilter_get_by_name("atempo"), "atempo", speed_param, nullptr, filter_graph);
    //                        avfilter_link(previous_filter, 0, tempo_filter, 0);
    //                        previous_filter = tempo_filter;
    //                    }
    //                }

    //                snprintf(speed_param, sizeof(speed_param), "%f", qPow(base, speedlog));
    //                last_filter = nullptr;
    //                avfilter_graph_create_filter(&last_filter, avfilter_get_by_name("atempo"), "atempo", speed_param, nullptr, filter_graph);
    //                avfilter_link(previous_filter, 0, last_filter, 0);
    //                //				}

    //                avfilter_link(last_filter, 0, buffersink_ctx, 0);
    //            } else {
    //                target_sample_rate = qRound64(target_sample_rate / playback_speed);
    //                avfilter_link(buffersrc_ctx, 0, buffersink_ctx, 0);
    //            }

    //            int sample_rates[] = { target_sample_rate, 0 };
    //            if (av_opt_set_int_list(buffersink_ctx, "sample_rates", sample_rates, 0, AV_OPT_SEARCH_CHILDREN) < 0) {
    //                dout << "[ERROR] Could not set output sample rates";
    //            }

    //            avfilter_graph_config(filter_graph, nullptr);

    //            audio_playback.reset = true;
    //        }

    //        media_handling.frame = av_frame_alloc();
    //    }

    //    for (int i=0;i<effects.size();i++) {
    //        effects.at(i)->open();
    //    }

    //    finished_opening = true;

    //    qInfo() << "Clip opened on track" << timeline_info.track;
    return true;
}


void ClipCacher::uninitialise()
{

}


void ClipCacher::worker(const long playhead, const bool resetCache, const bool scrubbing,
                        QVector<std::shared_ptr<Clip>>& nests)
{
    if (resetCache) {
        // note: for video, playhead is in "internal clip" frames - for audio, it's the timeline playhead
        reset(playhead);
        //        audio_playback.reset = false;
    }

    //    if (timeline_info.media == nullptr) {
    //        if (timeline_info.track >= 0) {
    //            cache_audio_worker(scrubbing, nests);
    //        }
    //    } else if (timeline_info.media->get_type() == MediaType::FOOTAGE) {
    //        if (media_handling.stream != nullptr) {
    //            if (media_handling.stream->codecpar != nullptr) {
    //                if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    //                    cache_video_worker(playhead);
    //                } else if (media_handling.stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
    //                    cache_audio_worker(scrubbing, nests);
    //                }
    //            }
    //        }
    //    }
}

void ClipCacher::reset(const long playhead)
{

}


