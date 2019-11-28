/* 
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
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
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "audiomonitor.h"

#include <QPainter>
#include <QLinearGradient>
#include <QtMath>
#include <limits>

#include "project/sequence.h"
#include "playback/audio.h"
#include "panels/panelmanager.h"

extern "C" {
#include "libavformat/avformat.h"
}

constexpr int AUDIO_MONITOR_GAP = 3;
constexpr auto PEAK_COLOUR = Qt::lightGray;
constexpr auto PEAK_MAXED_COLOUR = Qt::red;
constexpr auto PEAK_WIDTH = 2;

AudioMonitor::AudioMonitor(QWidget *parent) : QWidget(parent)
{
  reset();
}

void AudioMonitor::reset()
{
  sample_cache_offset = -1;
  sample_cache.clear();
  update();
}


void AudioMonitor::resetPeaks()
{
  peaks_.clear();
}

void AudioMonitor::resizeEvent(QResizeEvent* event)
{
  event->accept();
  gradient = QLinearGradient(QPoint(0, rect().top()), QPoint(0, rect().bottom()));
  gradient.setColorAt(0, Qt::red);
  gradient.setColorAt(0.25, Qt::yellow);
  gradient.setColorAt(1, Qt::green);
  QWidget::resizeEvent(event);
}

void AudioMonitor::paintEvent(QPaintEvent* event)
{
  event->accept();
  if (global::sequence == nullptr) {
    return;
  }

  QPainter p(this);
  const int channel_count = av_get_channel_layout_nb_channels(static_cast<uint64_t>(global::sequence->audioLayout()));

  const int channel_width = (width() / channel_count) - AUDIO_MONITOR_GAP;
  int32_t playhead_offset = -1;
  int channel_x = AUDIO_MONITOR_GAP;

  for (auto channel = 0; channel < channel_count; ++channel) {
    QRect r(channel_x, AUDIO_MONITOR_GAP, channel_width, height());
    p.fillRect(r, gradient);

    qint16 peak = -1;
    if ( (sample_cache_offset != -1) && !sample_cache.empty()) {
      playhead_offset = static_cast<int32_t>((global::sequence->playhead_ - sample_cache_offset) * channel_count);
      if ( (playhead_offset >= 0) && (playhead_offset < sample_cache.size()) ) {
        const qint16 val = sample_cache.at(playhead_offset + channel);
        const double multiplier = 1 - qAbs(static_cast<double>(val) / std::numeric_limits<int16_t>::max());
        const auto height = static_cast<qint16>(qRound(r.height() * multiplier));
        r.setHeight(height);
        peak = updatePeak(channel, height);
      } else {
        reset();
      }
    } else if (peaks_.count(channel) == 1) {
      // happens on a reset
      peak = peaks_.at(channel);
    }
    p.fillRect(r, QColor(0, 0, 0, 160));

    if (peak >= 0) {
      const QRect peak_rect(channel_x, peak, channel_width, PEAK_WIDTH);
      const QColor clr(peak > 0 ? PEAK_COLOUR : PEAK_MAXED_COLOUR);
      p.fillRect(peak_rect, clr);
    }

    channel_x += channel_width + AUDIO_MONITOR_GAP;
  }

  if (playhead_offset > -1) {
    // clean up used samples
    bool error = false;
    while ( (sample_cache_offset < global::sequence->playhead_) && !error) {
      sample_cache_offset++;
      for (auto i = 0; i < channel_count; i++) {
        if (sample_cache.isEmpty()) {
          reset();
          error = true;
          break;
        } else {
          sample_cache.removeFirst();
        }
      }//for
    }//while
  }
}


qint16 AudioMonitor::updatePeak(const int channel, const qint16 value)
{
  if (peaks_.count(channel) == 1) {
    if (peaks_[channel] > value) {
      peaks_[channel] = value;
    }
  } else {
    peaks_[channel] = value;
  }
  return peaks_[channel];
}
