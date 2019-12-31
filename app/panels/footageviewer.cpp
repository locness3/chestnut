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

#include <QThread>

#include "defaults.h"

using chestnut::panels::FootageViewer;
using chestnut::system::AudioWorker;
namespace mh = media_handling;

FootageViewer::FootageViewer(QWidget* parent)
  : ViewerBase(parent),
    audio_thread_(new QThread)
{
  audio_thread_->setPriority(QThread::HighPriority);
  audio_worker_ = new AudioWorker;
  audio_worker_->moveToThread(audio_thread_);
  connect(audio_thread_, &QThread::started, audio_worker_, &AudioWorker::process);
  connect(audio_worker_, &AudioWorker::finished, audio_thread_, &QThread::quit);
  connect(audio_worker_, &AudioWorker::finished, audio_worker_, &AudioWorker::deleteLater);
  connect(audio_thread_, &QThread::finished, audio_worker_, &AudioWorker::deleteLater);
  audio_thread_->start();
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
    if (!tracks.empty() && (tracks.front() != nullptr) ) {
      video_track_ = tracks.front();
      time_.scale_ = video_track_->timescale_;
      time_.interval_ = qRound(1000.0 / video_track_->video_frame_rate);
    }
    tracks = footage_->audioTracks();
    if (!tracks.empty() && (tracks.front() != nullptr)) {
      audio_track_ = tracks.front();
      Q_ASSERT(audio_worker_);
      audio_worker_->setup(audio_track_->audio_channels);
    }
    if ( (audio_track_ == nullptr) || (video_track_ == nullptr) ) {
      return;
    }
    setupTimer(time_.interval_);
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
  time_.stamp_ += qRound((static_cast<double>(time_.interval_/1000.0) / time_.scale_.numerator()) * time_.scale_.denominator());
  if (video_track_) {
    const auto [pm, ts] = video_track_->frame();
    if (pm.isNull()) {
      end_of_stream_ = true;
      pause();
      return;
    }
    current_timecode_->set_value(time_.stamp_, false);
    frame_view_->setPixmap(pm.scaledToHeight(frame_view_->height(), Qt::SmoothTransformation));
    if (audio_track_) {
      audio_worker_->queueData(audio_track_->audioFrame(time_.stamp_));
    }
  } else if (audio_track_) {
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
  time_.stamp_ = 0;
  video_track_->seek(0);
  nextFrame();
}

void FootageViewer::seekToEnd()
{
  //TODO:
}

void FootageViewer::seekToInPoint()
{
  if (footage_->using_inout) {
    video_track_->seek(footage_->in);
    time_.stamp_ = footage_->in;
    nextFrame();
  } else {
    seekToStart();
  }
}

void FootageViewer::seekToOutPoint()
{
  if (footage_->using_inout) {
    video_track_->seek(footage_->out);
    nextFrame();
  } else {
    seekToEnd();
  }
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
  nextFrame();
}

void FootageViewer::setupTimer(const int32_t interval)
{
  qDebug() << "Interval" << interval;
  play_timer_.stop();
  play_timer_.setInterval(interval);
  play_timer_.setTimerType(Qt::TimerType::CoarseTimer);
  play_timer_.setSingleShot(false);
  connect(&play_timer_, &QTimer::timeout, this, &FootageViewer::onTimeout);
}

void FootageViewer::setPlayPauseIcon(const bool playing)
{
  play_btn_->setIcon(playing ? play_icon_ : QIcon(":/icons/pause.png"));
}
