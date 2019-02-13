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
#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include <QThread>
#include <QOffscreenSurface>
#include <QMutex>
#include <QWaitCondition>

class ExportDialog;
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct AVStream;
struct AVCodec;
struct SwsContext;
struct SwrContext;

extern "C" {
#include <libavcodec/avcodec.h>
}

#define COMPRESSION_TYPE_CBR 0
#define COMPRESSION_TYPE_CFR 1
#define COMPRESSION_TYPE_TARGETSIZE 2
#define COMPRESSION_TYPE_TARGETBR 3

class ExportThread : public QThread {
    Q_OBJECT
public:
    ExportThread();

    ExportThread(const ExportThread& ) = delete;
    ExportThread& operator=(const ExportThread&) = delete;

    // export parameters
    QString filename;
    bool video_enabled;
    int video_codec;
    int video_width;
    int video_height;
    double video_frame_rate;
    int video_compression_type;
    double video_bitrate;
    bool audio_enabled;
    int audio_codec;
    int audio_sampling_rate;
    int audio_bitrate;
    int32_t start_frame;
    int32_t end_frame;

    QOffscreenSurface surface;

    ExportDialog* ed;

    bool continueEncode = true;
  protected:
    void run() override;
signals:
    void progress_changed(int value, qint64 remaining_ms);
public slots:
    void wake();
private:
    bool encode(AVFormatContext* ofmt_ctx, AVCodecContext* codec_ctx, AVFrame* frame,
                AVPacket* packet, AVStream* stream, bool rescale);
    bool setupVideo();
    bool setupAudio();
    bool setupContainer();

    AVFormatContext* fmt_ctx = nullptr;
    AVStream* video_stream= nullptr;
    AVCodec* vcodec= nullptr;
    AVCodecContext* vcodec_ctx= nullptr;
    AVFrame* video_frame= nullptr;
    AVFrame* sws_frame= nullptr;
    SwsContext* sws_ctx= nullptr;
    AVStream* audio_stream= nullptr;
    AVCodec* acodec= nullptr;
    AVFrame* audio_frame= nullptr;
    AVFrame* swr_frame= nullptr;
    AVCodecContext* acodec_ctx= nullptr;
    AVPacket video_pkt;
    AVPacket audio_pkt;
    SwrContext* swr_ctx = nullptr;

    bool vpkt_alloc;
    bool apkt_alloc;

    int aframe_bytes;
    int ret;

    QMutex mutex;
    QWaitCondition waitCond;
};

#endif // EXPORTTHREAD_H
