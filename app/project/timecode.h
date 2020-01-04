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
      TimeCode(media_handling::Rational time_scale, media_handling::Rational frame_rate, const int64_t time_stamp=0);
      int64_t toMillis() const;
      std::string toString() const;
      int64_t toFrames() const;
      void setTimestamp(const int64_t time_stamp) noexcept;
      media_handling::Rational timeScale() const noexcept;
      media_handling::Rational frameRate() const noexcept;
      int64_t timestamp() const noexcept;

    private:
      media_handling::Rational time_scale_;
      media_handling::Rational frame_rate_;
      int64_t time_stamp_ {0};
      struct {
        bool drop_;
        int32_t second_;
        int32_t minute_;
        int32_t hour_;
      } frames_;
  };
}



#endif // TIMECODE_H
