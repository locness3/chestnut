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

#include <QtMath>

#include "panels/panelmanager.h"

using chestnut::ui::TimelineBase;
using panels::PanelManager;

TimelineBase::TimelineBase()
{

}


int64_t TimelineBase::getScreenPointFromFrame(const double zoom, const int64_t frame) const noexcept
{
  return static_cast<int>(qFloor(static_cast<double>(frame) * zoom));
}


bool TimelineBase::snapToPoint(const int64_t point, int64_t& l) noexcept
{
  const auto limit = snapRange();
  if ( (l > (point - limit - 1)) && (l < (point + limit + 1)) ) {
    snap_.point_ = point;
    l = point;
    snap_.active_ = true;
    return true;
  }
  return false;
}

bool TimelineBase::snapToTimeline(int64_t& l, const bool use_playhead, const bool use_markers, const bool use_workarea)
{
  auto item = viewed_item_.lock();
  Q_ASSERT(item);
  snap_.active_ = false;
  if (!snap_.enabled_) {
    return false;
  }
    if (use_playhead && !PanelManager::sequenceViewer().playing) {
      // snap to playhead
      if (snapToPoint(item->playhead(), l)) {
        return true;
      }
    }

    // snap to marker
    if (use_markers) {
      for (const auto& mark : item->markers_) {
        if (mark != nullptr && snapToPoint(mark->frame, l)) {
          return true;
        }
      }
    }

    // snap to in/out
    if (use_workarea && item->workareaActive()) {
      if (snapToPoint(item->inPoint(), l)) {
        return true;
      }
      if (snapToPoint(item->outPoint(), l)) {
        return true;
      }
    }

    // snap to clip/transition
    // TODO:
//    for (const auto& c : item->clips()) {
//      if (c == nullptr) {
//        continue;
//      }
//      if (snapToPoint(c->timeline_info.in, l)) {
//        return true;
//      } else if (snapToPoint(c->timeline_info.out, l)) {
//        return true;
//      } else if (c->getTransition(ClipTransitionType::OPENING) != nullptr
//                 && snapToPoint(c->timeline_info.in
//                                  + c->getTransition(ClipTransitionType::OPENING)->get_true_length(), l)) {
//        return true;
//      } else if (c->getTransition(ClipTransitionType::CLOSING) != nullptr
//                 && snapToPoint(c->timeline_info.out
//                                  - c->getTransition(ClipTransitionType::CLOSING)->get_true_length(), l)) {
//        return true;
//      }
//    }
    return false;
}

int64_t TimelineBase::getFrameFromScreenPoint(const double zoom, const int x) const noexcept
{
  if (const auto f = qCeil(static_cast<double>(x) / zoom); f < 0) {
    return 0;
  } else {
    return f;
  }
}


int64_t TimelineBase::snapRange() const noexcept
{
  return getFrameFromScreenPoint(zoom_, 10);
}
