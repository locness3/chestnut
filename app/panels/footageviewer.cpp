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

#include "footageviewer.h"

using chestnut::panels::FootageViewer;
namespace mh = media_handling;

FootageViewer::FootageViewer(QWidget* parent)
  : ViewerBase(parent)
{
}

void FootageViewer::clear()
{
  frame_view_->clear();
  disconnect(&play_timer_, &QTimer::timeout, this, &FootageViewer::onTimeout);
  pause();
}

void FootageViewer::setMedia(MediaWPtr mda)
{
  clear();
  ViewerBase::setMedia(std::move(mda));
  if (auto mda_item = media_.lock()) {
    if (mda_item->type() != MediaType::FOOTAGE) {
      return;
    }
    footage_ = mda_item->object<Footage>();
    if (footage_ == nullptr) {
      return;
    }
    auto tracks = footage_->videoTracks();
    if (tracks.empty() || (tracks.front() == nullptr) ) {
      return;
    }
    video_track_ = tracks.front();
    setupTimer(video_track_->video_frame_rate);
    current_timecode_->set_frame_rate(video_track_->video_frame_rate);
    seekToInPoint();
    nextFrame();
  }
}


void FootageViewer::seek(const int64_t position)
{

}

void FootageViewer::play()
{
  if (end_of_stream_) {
    seekToInPoint();
    end_of_stream_ = false;
  }
  setPlayPauseIcon(false);
  nextFrame();
  playing_ = true;
  play_timer_.start();
  emit started();
}

void FootageViewer::pause()
{
  setPlayPauseIcon(true);
  playing_ = false;
  play_timer_.stop();
  emit stopped();
}

bool FootageViewer::isPlaying() const
{
  return playing_;
}

void FootageViewer::zoomIn()
{
  //TODO:
}

void FootageViewer::zoomOut()
{
  //TODO:
}

void FootageViewer::togglePointsEnabled()
{
  //TODO:
}

void FootageViewer::setInPoint()
{
  //TODO:
}
void FootageViewer::setOutPoint()
{
  //TODO:
}

void FootageViewer::clearInPoint()
{
  //TODO:
}

void FootageViewer::clearOutPoint()
{
  //TODO:
}

void FootageViewer::clearPoints()
{
  //TODO:
}

bool FootageViewer::isFocused() const
{
  //TODO:
  return false;
}


void FootageViewer::nextFrame()
{
  if (video_track_) {
    const auto pm = video_track_->frame();
    if (pm.isNull()) {
      end_of_stream_ = true;
      pause();
    }
    frame_view_->setPixmap(pm.scaledToHeight(frame_view_->height()));
  } else if (!audio_tracks_.empty()) {
    // TODO: draw waveform
  } else {
    qWarning() << "No audio or video footage";
    return;
  }
}

void FootageViewer::previousFrame()
{
  //TODO:
}


void FootageViewer::seekToStart()
{
  //TODO:
}

void FootageViewer::seekToEnd()
{
  //TODO:
}
void FootageViewer::seekToInPoint()
{
  const auto in = footage_->using_inout ? footage_->in : 0;
  video_track_->seek(in);
}

void FootageViewer::seekToOutPoint()
{
  //TODO:
}

void FootageViewer::updatePanelTitle()
{
  QString item_name;
  if (auto mda = media_.lock()) {
    item_name = mda->name();
  } else {
    item_name = "(none)";
  }
  setWindowTitle(QString("%1: %2").arg(name_, item_name));
}


void FootageViewer::onTimeout()
{
  qDebug() << play_timer_.interval();
  nextFrame();
}

void FootageViewer::setupTimer(const double frame_rate)
{
  Q_ASSERT(!qFuzzyCompare(frame_rate, 0.0));
  play_timer_.stop();
  play_timer_.setInterval(qRound(1000.0 / frame_rate));
  play_timer_.setTimerType(Qt::TimerType::CoarseTimer);
  play_timer_.setSingleShot(false);
  connect(&play_timer_, &QTimer::timeout, this, &FootageViewer::onTimeout);
}

void FootageViewer::setPlayPauseIcon(const bool playing)
{
  play_btn_->setIcon(playing ? play_icon_ : QIcon(":/icons/pause.png"));
}

