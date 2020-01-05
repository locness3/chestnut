/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019 Jonathan Noble
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

#ifndef TIMECODE_H
#define TIMECODE_H

#include <mediahandling/types.h>
#include <string>

namespace chestnut::project
{
  class TimeCode
  {
    public:
      /**
       * @brief Constructor
       * @param time_scale
       * @param frame_rate
       * @param time_stamp
       */
      TimeCode(media_handling::Rational time_scale, media_handling::Rational frame_rate, const int64_t time_stamp=0);
      /**
       * @brief Convert time to millis
       * @return
       */
      int64_t toMillis() const;
      /**
       * @brief Convert to SMPTE timecode format
       * @param drop  Use drop-frame timecode if possible
       * @return  SMPTE timecode
       */
      std::string toString(const bool drop=true) const;
      /**
       * @brief Convert time to number of frames
       * @return
       */
      int64_t toFrames() const;
      /**
       * @brief             Set the time in units of time-scale
       * @param time_stamp  Value to set
       */
      void setTimestamp(const int64_t time_stamp) noexcept;
      /**
       * @brief Retrieve this time's scale
       * @return
       */
      media_handling::Rational timeScale() const noexcept;
      /**
       * @brief   Retrieve this time's frame rate
       * @return
       */
      media_handling::Rational frameRate() const noexcept;
      /**
       * @brief Retrieve this time's time-stamp
       * @return
       */
      int64_t timestamp() const noexcept;
    private:
      friend class TimeCodeTest;
      media_handling::Rational time_scale_;
      media_handling::Rational frame_rate_;
      int64_t time_stamp_ {0};
      struct {
          bool drop_ {false};
          int32_t second_ {0};
          int32_t minute_ {0};
          int32_t drop_minute_ {0};
          int32_t ten_minute_ {0};
          int32_t drop_ten_minute_ {0};
          int32_t hour_ {0};
          int32_t drop_count_ {0};
      } frames_;
      /**
       * @brief         Convert frames to smpte timecode
       * @param frames
       * @param drop    true==use drop-frame format
       * @return        hh:mm:ss:;ff
       */
      std::string framesToSMPTE(int64_t frames, const bool drop=true) const;

  };
}



#endif // TIMECODE_H
