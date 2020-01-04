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

#include "timecode.h"

#include <fmt/core.h>
#include <cmath>

using chestnut::project::TimeCode;

constexpr auto NON_DROP_FMT = "{:02d}:{:02d}:{:02d}:{:02d}";
constexpr auto DROP_FMT = "{:02d}:{:02d}:{:02d};{:02d}";

namespace
{
  const media_handling::Rational NTSC_30 {30000, 1001};
  const media_handling::Rational NTSC_60 {60000, 1001};
}


TimeCode::TimeCode(media_handling::Rational time_scale, media_handling::Rational frame_rate, const int64_t time_stamp)
  : time_scale_(std::move(time_scale)),
    frame_rate_(std::move(frame_rate)),
    time_stamp_(time_stamp)
{
  frames_.drop_ = (frame_rate_ == NTSC_30) || (frame_rate_ == NTSC_60);
  frames_.second_ = lround(frame_rate_.toDouble());
  frames_.minute_ = lround((frame_rate * 60).toDouble());
  frames_.hour_ = lround((frame_rate * 3600).toDouble());
}

int64_t TimeCode::toMillis() const
{
  return llround((time_stamp_ * time_scale_).toDouble() * 1000);
}

std::string TimeCode::toString() const
{
  const auto frames = toFrames();
  const auto f_rem = frames % frames_.second_;
  const auto s = (frames / frames_.second_) % 60;
  const auto m = (frames / frames_.minute_) % 60;
  const auto h = (frames / frames_.hour_) % 60;
  if (frames_.drop_) {
    return {};
  }
  return fmt::format(NON_DROP_FMT, h, m, s, f_rem);
}

int64_t TimeCode::toFrames() const
{
  return floor(((time_stamp_ * time_scale_) * frame_rate_).toDouble());
}

void TimeCode::setTimestamp(const int64_t time_stamp) noexcept
{
  time_stamp_ = time_stamp;
}


media_handling::Rational TimeCode::timeScale() const noexcept
{
  return time_scale_;
}

media_handling::Rational TimeCode::frameRate() const noexcept
{
  return frame_rate_;
}

int64_t TimeCode::timestamp() const noexcept
{
  return time_stamp_;
}



