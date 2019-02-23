#ifndef TIMELINEINFO_H
#define TIMELINEINFO_H

#include <QString>
#include <QColor>
#include <atomic>
#include <memory>

#include "project/media.h"


namespace project {
  /**
   * @brief The TimelineInfo class of whose members may be used in multiple threads
   */
  class TimelineInfo
  {
    public:
      TimelineInfo();
      bool isVideo() const;

      bool enabled = true;
      long clip_in = 0;
      long in = 0;
      long out = 0;
      std::atomic_int32_t track_{-1};
      QString name = "";
      QColor color = {0,0,0};
      MediaPtr media = nullptr;
      int media_stream = -1;
      std::atomic<double> speed{1.0};
      double cached_fr = 0.0;
      bool reverse = false;
      bool maintain_audio_pitch = true;
      bool autoscale = true;
  };
  using TimelineInfoPtr = std::shared_ptr<TimelineInfo>;
}



#endif // TIMELINEINFO_H
