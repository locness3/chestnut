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

#include "timelinebase.h"

#include <cmath>

using chestnut::ui::TimelineBase;

TimelineBase::TimelineBase()
{

}


int TimelineBase::getScreenPointFromFrame(const double zoom, const int64_t frame) const noexcept
{
  return static_cast<int>(std::floor(static_cast<double>(frame) * zoom));
}
