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

    static std::atomic_bool exporting;

    // export parameters
    QString filename;
    struct {
        bool enabled = false;
        int codec = -1;
        int width = -1;
        int height = -1;
        double frame_rate = 0.0;
        int compression_type = -1;
        double bitrate = 0.0;
    } video_params;

    struct {
      bool enabled = false;
      int codec = -1;
      int sampling_rate = -1;
      int bitrate = -1;
    } audio_params;

    int64_t start_frame;
    int64_t end_frame;

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

    bool vpkt_alloc = false;
    bool apkt_alloc = false;

    int aframe_bytes;
    int ret;

    QMutex mutex;
    QWaitCondition waitCond;
};

#endif // EXPORTTHREAD_H
