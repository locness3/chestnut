#ifndef MEDIAHANDLER_H
#define MEDIAHANDLER_H

#include <libavformat/avformat.h>


namespace project
{

  class MediaHandler
  {
    public:
      MediaHandler() = default;
      ~MediaHandler();

      AVFormatContext* format_ctx_ = nullptr;
      AVStream* stream_ = nullptr;
      AVCodec* codec_ = nullptr;
      AVCodecContext* codec_ctx_ = nullptr;
      AVPacket* packet_ = nullptr;
      AVFrame* frame_ = nullptr;
      AVDictionary* opts_ = nullptr;
      long calculated_length_ = -1;
  };
}

#endif // MEDIAHANDLER_H
