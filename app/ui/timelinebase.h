/*
 * Chestnut. Chestnut is a free non-linear video editor.
 * Copyright (C) 2020 Jon Noble
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

#ifndef TIMELINEBASE_H
#define TIMELINEBASE_H

#include <stdint.h>

#include "project/projectitem.h"

namespace chestnut::ui
{
  class TimelineBase
  {
    public:
      TimelineBase();
    protected:
      int32_t screenPointFromFrame(const double zoom, const int64_t frame) const noexcept;
      bool snapToPoint(const int64_t point, int64_t& l) noexcept;
      bool snapToTimeline(int64_t& l, const bool use_playhead, const bool use_markers, const bool use_workarea);
      int64_t frameFromScreenPoint(const double zoom, const int32_t x) const noexcept;
      QString timeCodeFromFrame(int64_t frame, const int view, const double frame_rate);

     private:
      friend class ViewerTimeline;
      project::ProjectItemWPtr viewed_item_;
      double zoom_;
      struct {
       /**
        * @brief Position of snap
        */
        int64_t point_ {-1};
        /**
         * @brief Is the current position snapped to place
         */
        bool enabled_ {false};
        /**
         * @brief Is the time line configured to snap
         */
        bool active_ {false};
      } snap_;

    private:
      int64_t snapRange() const noexcept;

  };
}

#endif // TIMELINEBASE_H
