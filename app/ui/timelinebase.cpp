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
#include "io/config.h"

using chestnut::ui::TimelineBase;
using panels::PanelManager;


TimelineBase::TimelineBase()
{

}


int32_t TimelineBase::screenPointFromFrame(const double zoom, const int64_t frame) const noexcept
{
  return static_cast<int32_t>(qFloor(frame * zoom));
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

int64_t TimelineBase::frameFromScreenPoint(const double zoom, const int32_t x) const noexcept
{
  if (const auto f = qCeil(static_cast<double>(x) / zoom); f < 0) {
    return 0;
  } else {
    return f;
  }
}


QString TimelineBase::timeCodeFromFrame(int64_t frame, const int view, const double frame_rate)
{
  // TODO: replace with media_handling::TimeCode
  if (qFuzzyCompare(frame_rate, 0)) {
    return "NaN";
  }
  if (view == TIMECODE_FRAMES) {
    return QString::number(frame);
  }

  // return timecode
  int hours = 0;
  int mins = 0;
  int secs = 0;
  int frames = 0;
  auto token = ':';

  if (( (view == TIMECODE_DROP) || (view == TIMECODE_MILLISECONDS) ) && frame_rate_is_droppable(frame_rate)) {
    //CONVERT A FRAME NUMBER TO DROP FRAME TIMECODE
    //Code by David Heidelberger, adapted from Andrew Duncan, further adapted for Olive by Olive Team
    //Given an int called framenumber and a double called framerate
    //Framerate should be 29.97, 59.94, or 23.976, otherwise the calculations will be off.

    const int dropFrames = qRound(frame_rate * 0.066666); //Number of frames to drop on the minute marks is the nearest integer to 6% of the framerate
    const int framesPerHour = qRound(frame_rate * 60 * 60); //Number of frqRound64ames in an hour
    const int framesPer24Hours = framesPerHour * 24; //Number of frames in a day - timecode rolls over after 24 hours
    const int framesPer10Minutes = qRound(frame_rate * 60 * 10); //Number of frames per ten minutes
    const int framesPerMinute = (qRound(frame_rate) * 60) -  dropFrames; //Number of frames per minute is the round of the framerate * 60 minus the number of dropped frames

    //If framenumber is greater than 24 hrs, next operation will rollover clock
    frame = frame % framesPer24Hours; //% is the modulus operator, which returns a remainder. a % b = the remainder of a/b

    const auto d = frame / framesPer10Minutes; // \ means integer division, which is a/b without a remainder. Some languages you could use floor(a/b)
    const auto m = frame % framesPer10Minutes;

    //In the original post, the next line read m>1, which only worked for 29.97. Jean-Baptiste Mardelle correctly pointed out that m should be compared to dropFrames.
    if (m > dropFrames) {
      frame = frame + (dropFrames*9*d) + dropFrames * ((m - dropFrames) / framesPerMinute);
    } else {
      frame = frame + dropFrames*9*d;
    }

    const int frRound = qRound(frame_rate);
    frames = frame % frRound;
    secs = (frame / frRound) % 60;
    mins = ((frame / frRound) / 60) % 60;
    hours = (((frame / frRound) / 60) / 60);

    token = ';';
  } else {
    // non-drop timecode

    const int int_fps = qRound(frame_rate);
    hours = frame / (3600 * int_fps);
    mins = frame / (60 * int_fps) % 60;
    secs = frame / int_fps % 60;
    frames = frame % int_fps;
  }
  if (view == TIMECODE_MILLISECONDS) {
    return QString::number((hours * 3600000) + (mins * 60000) + (secs * 1000) + qCeil(frames * 1000 / frame_rate));
  }
  return QString(QString::number(hours).rightJustified(2, '0') +
                 ":" + QString::number(mins).rightJustified(2, '0') +
                 ":" + QString::number(secs).rightJustified(2, '0') +
                 token + QString::number(frames).rightJustified(2, '0')
                 );
}


int64_t TimelineBase::snapRange() const noexcept
{
  return frameFromScreenPoint(zoom_, 10);
}
