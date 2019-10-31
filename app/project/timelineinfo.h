/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMELINEINFO_H
#define TIMELINEINFO_H

#include <QString>
#include <QColor>
#include <atomic>
#include <memory>

#include "project/media.h"
#include "project/ixmlstreamer.h"


namespace project {
  /**
   * @brief The TimelineInfo class of whose members may be used in multiple threads.
   */
  class TimelineInfo : public project::IXMLStreamer
  {
    public:
      bool enabled = true;
      std::atomic_int64_t clip_in{0};
      std::atomic_int64_t in{0};
      std::atomic_int64_t out{0};
      std::atomic_int32_t track_{-1};
      QString name_ = "";
      QColor color = {0,0,0};
      //TODO: make weak
      MediaPtr media = nullptr; //TODO: assess Media members in lieu of c++20 atomic_shared_ptr
      std::atomic_int32_t media_stream{-1};
      std::atomic<double> speed{1.0};
      double cached_fr = 0.0;
      bool reverse = false; //TODO: revisit
      bool maintain_audio_pitch = true;
      bool autoscale = true;

      TimelineInfo() = default;
      ~TimelineInfo() override = default;

      [[deprecated("Replaced by ClipType Clip::mediaType()")]]
      bool isVideo() const;
      TimelineInfo& operator=(const TimelineInfo& rhs);
      TimelineInfo& operator=(const TimelineInfo&&) = delete;
      TimelineInfo(const TimelineInfo&) = delete;
      TimelineInfo(const TimelineInfo&&) = delete;

      virtual bool load(QXmlStreamReader& stream) override;
      virtual bool save(QXmlStreamWriter& stream) const override;
  };
  using TimelineInfoPtr = std::shared_ptr<TimelineInfo>;
}



#endif // TIMELINEINFO_H
