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

#include "timelinetrackarea.h"
#include "ui_timelinetrackarea.h"
#include "debug.h"

TimelineTrackArea::TimelineTrackArea(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TimelineTrackArea)
{
  ui->setupUi(this);
}

TimelineTrackArea::~TimelineTrackArea()
{
  delete ui;
}


bool TimelineTrackArea::initialise(const TimelineTrackType area_type)
{
  type_ = area_type;
  return true;
}


bool TimelineTrackArea::addTrack(const int number, const QString& name)
{
  if (track_widgets_.contains(number)) {
    qWarning() << "Already added track" << number;
    return false;
  }

  auto idx = number;
  if ( (type_ == TimelineTrackType::VISUAL) && (idx >= 0) ) {
    // ensure video track numbers are negative and start from -1. needed for display purposes
    idx = -(idx + 1);
  }
  auto track_wdgt = new TrackAreaWidget(this);
  track_wdgt->initialise();
  track_wdgt->setName(name);
  QObject::connect(track_wdgt, &TrackAreaWidget::enableTrack, this, &TimelineTrackArea::trackEnableChange);
  QObject::connect(track_wdgt, &TrackAreaWidget::lockTrack, this, &TimelineTrackArea::trackLockChange);
  track_widgets_[idx] = track_wdgt;
  update();
  return true;
}


void TimelineTrackArea::setHeights(const QVector<int>& heights)
{
  for (auto ix = 0; ix < heights.size(); ++ix) {
    const auto idx = type_ == TimelineTrackType::VISUAL ? -(ix + 1) : ix; // negative offset for video only to get display correct
    if (track_widgets_.contains(idx) && track_widgets_[idx] != nullptr)  {
      // fixed height
      track_widgets_[idx]->setMinimumHeight(heights.at(ix));
      track_widgets_[idx]->setMaximumHeight(heights.at(ix));
    }
  }
}


void TimelineTrackArea::reset()
{

  for (auto t_w : track_widgets_) {
    ui->mainLayout->removeWidget(t_w);
    delete t_w;
  }
  track_widgets_.clear();
}

void TimelineTrackArea::trackEnableChange(const bool enabled)
{
  for (auto key_val : track_widgets_.toStdMap()) {
    if (key_val.second == sender()) {
      qInfo() << key_val.first << enabled;
      emit trackEnabled(enabled, key_val.first);
      return;
    }
  }
  qWarning() << "No widget found";
}

void TimelineTrackArea::trackLockChange(const bool locked)
{
  for (auto key_val : track_widgets_.toStdMap()) {
    if (key_val.second == sender()) {
      qInfo() << key_val.first << locked;
      emit trackLocked(locked, key_val.first);
      return;
    }
  }
  qWarning() << "No widget found";
}


void TimelineTrackArea::update()
{
  for (auto key_val : track_widgets_.toStdMap()) {
    ui->mainLayout->insertWidget(key_val.first, key_val.second);
  }
}
