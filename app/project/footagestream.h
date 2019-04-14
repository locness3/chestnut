#ifndef FOOTAGESTREAM_H
#define FOOTAGESTREAM_H

#include <memory>
#include <QImage>
#include <QIcon>

#include "project/ixmlstreamer.h"

namespace project {

  enum class ScanMethod {
    PROGRESSIVE = 0,
    TOP_FIRST = 1,
    BOTTOM_FIRST = 2,
    UNKNOWN
  };

  enum class StreamType{
    AUDIO,
    VIDEO,
    UNKNOWN
  };

  class FootageStream : public project::IXMLStreamer {
    public:
      FootageStream() = default;

      void make_square_thumb();
      virtual void load(const QXmlStreamReader& stream) override;
      virtual bool save(QXmlStreamWriter& stream) const override;

      int file_index = -1;
      int video_width = -1;
      int video_height = -1;
      bool infinite_length = false;
      double video_frame_rate = 0.0;
      ScanMethod video_interlacing = ScanMethod::UNKNOWN;
      ScanMethod video_auto_interlacing = ScanMethod::UNKNOWN;
      int audio_channels = -1;
      int audio_layout = -1;
      int audio_frequency = -1;
      bool enabled = false;

      // preview thumbnail/waveform
      bool preview_done = false;
      QImage video_preview;
      QIcon video_preview_square;
      QVector<char> audio_preview;
      StreamType type_{StreamType::UNKNOWN};
  };
  using FootageStreamPtr = std::shared_ptr<FootageStream>;
  using FootageStreamWPtr = std::weak_ptr<FootageStream>;
}

#endif // FOOTAGESTREAM_H
